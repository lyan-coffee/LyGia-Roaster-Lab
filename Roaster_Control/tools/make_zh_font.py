#!/usr/bin/env python3
# Tao font tieng Hoa NHO cho man hinh (chi cac chu dang dung trong lib/RoasterI18n.h) - KHONG can lv_font_conv / Node.js.
# Can: Python 3 + Pillow (pip install pillow) + 1 file font co chu Hoa, vi du NotoSansSC-Regular.ttf.
#   python tools/make_zh_font.py NotoSansSC-Regular.ttf
# Ket qua: lib/lv_font_zh_14.h, lib/lv_font_zh_16.h, lib/lv_font_zh_20.h (lib/RoasterFontZh.h tu include 3 file nay).
# Sau do dat ENABLE_ZH_FONT 1 trong LySetting.h va bien dich lai. Them tu moi vao the Chinese thi chay lai script.
# Dinh dang dau ra: LVGL 9 (lv_font_fmt_txt), bpp 4, khong nen, khong kerning. Chu ASCII / bieu tuong lay tu font Montserrat (fallback).
import os, sys
from PIL import ImageFont

root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
if len(sys.argv) < 2:
    sys.exit("Dung: python tools/make_zh_font.py NotoSansSC-Regular.ttf")
fontfile = sys.argv[1]
src = open(os.path.join(root, "lib", "RoasterI18n.h"), encoding="utf-8").read()
EXTRA = "中文"                                               # chu khong nam trong chuoi dich: ten ngon ngu tren nut Language
chars = sorted({c for c in src if ord(c) >= 0x2E80} | set(EXTRA))
print("So chu:", len(chars), "".join(chars))

def build(size):
    f = ImageFont.truetype(fontfile, size)
    asc, desc = f.getmetrics()
    bitmap = bytearray(); dsc = []
    for ch in chars:
        l, t, r, b = f.getbbox(ch, anchor="ls")             # toa do so voi duong co so (y xuong duoi)
        w, h = max(r - l, 0), max(b - t, 0)
        m = f.getmask(ch, mode="L")
        mw, mh = m.size
        px = [0] * (w * h)
        # mask co the lech goc voi bbox: lay theo offset cua mask
        ox, oy = m.getbbox()[0:2] if m.getbbox() else (0, 0)
        data = bytes(m)
        for y in range(min(h, mh)):
            for x in range(min(w, mw)):
                px[y * w + x] = data[y * mw + x] >> 4
        idx = len(bitmap)
        for i in range(0, len(px), 2):                      # 2 diem anh / byte, nibble cao truoc
            bitmap.append((px[i] << 4) | (px[i + 1] if i + 1 < len(px) else 0))
        dsc.append((idx, int(round(f.getlength(ch) * 16)), w, h, l, -b))
    return asc, desc, bitmap, dsc

for size in (14, 16, 20):
    asc, desc, bitmap, dsc = build(size)
    cps = [ord(c) for c in chars]
    start = cps[0]; length = cps[-1] - start + 1
    o = []
    o.append("/* Tu dong sinh boi tools/make_zh_font.py - KHONG sua tay. Font: %s, %d px, 4 bpp */\n#pragma once\n" % (os.path.basename(fontfile), size))
    o.append('#ifdef LV_LVGL_H_INCLUDE_SIMPLE\n    #include "lvgl.h"\n#else\n    #include <lvgl.h>\n#endif\n\n')
    P = "zh%d_" % size
    o.append("static LV_ATTRIBUTE_LARGE_CONST const uint8_t %sglyph_bitmap[] = {\n" % P)
    for i in range(0, len(bitmap), 16):
        o.append("    " + ", ".join("0x%02x" % v for v in bitmap[i:i + 16]) + ",\n")
    o.append("};\n\nstatic const lv_font_fmt_txt_glyph_dsc_t %sglyph_dsc[] = {\n" % P)
    o.append("    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,\n")
    for (bi, adv, w, h, ox, oy) in dsc:
        o.append("    {.bitmap_index = %d, .adv_w = %d, .box_w = %d, .box_h = %d, .ofs_x = %d, .ofs_y = %d},\n" % (bi, adv, w, h, ox, oy))
    o.append("};\n\nstatic const uint16_t %sunicode_list_0[] = {\n" % P)
    offs = [c - start for c in cps]
    for i in range(0, len(offs), 12):
        o.append("    " + ", ".join("0x%x" % v for v in offs[i:i + 12]) + ",\n")
    o.append("};\n\nstatic const lv_font_fmt_txt_cmap_t %scmaps[] = {\n    {\n" % P)
    o.append("        .range_start = %d, .range_length = %d, .glyph_id_start = 1,\n" % (start, length))
    o.append("        .unicode_list = %sunicode_list_0, .glyph_id_ofs_list = NULL, .list_length = %d, .type = LV_FONT_FMT_TXT_CMAP_SPARSE_TINY\n    }\n};\n\n" % (P, len(cps)))
    o.append("static const lv_font_fmt_txt_dsc_t %sfont_dsc = {\n    .glyph_bitmap = %sglyph_bitmap,\n    .glyph_dsc = %sglyph_dsc,\n    .cmaps = %scmaps,\n    .kern_dsc = NULL,\n    .kern_scale = 0,\n    .cmap_num = 1,\n    .bpp = 4,\n    .kern_classes = 0,\n    .bitmap_format = 0,\n};\n\n" % (P, P, P, P))
    o.append("const lv_font_t lv_font_zh_%d = {\n    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,\n    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,\n    .line_height = %d,\n    .base_line = %d,\n    .subpx = LV_FONT_SUBPX_NONE,\n    .underline_position = -1,\n    .underline_thickness = 1,\n    .dsc = &%sfont_dsc\n};\n" % (size, asc + desc, desc, P))
    out = os.path.join(root, "lib", "lv_font_zh_%d.h" % size)
    open(out, "w", encoding="utf-8").write("".join(o))
    print("Da ghi", out, "(%d byte bitmap)" % len(bitmap))
