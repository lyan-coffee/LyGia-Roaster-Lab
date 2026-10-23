#!/usr/bin/env python3
"""
make_icons.py - sinh bieu tuong 34 px (RGB565A8) cho lib/RoasterIcons_std.h
  Bieu tuong Heater / Air / Drum / Cool: ve tu SVG ben duoi (doi mau o day neu doi quy tac mau).
  Huy hieu HiBean / Artisan: tu anh PNG tron 136 px (badge_*_136.png, lam tu logo goc logo_*_src.png) thu nho xuong 34 px.
  Can: python3, pillow, playwright (chromium).   Chay:  python3 tools/make_icons.py
  Kich thuoc lay theo UI_ICON trong lib/UIConfig.h (mac dinh 34).
"""
import os, re, io
from PIL import Image
HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
cfg = open(os.path.join(ROOT, 'lib', 'UIConfig.h')).read()
PX = int(re.search(r'#define\s+UI_ICON\s+(\d+)', cfg).group(1))
SC = 4

FLAME = 'M19.48 12.35c-1.57-4.08-7.16-4.3-5.81-10.23.1-.44-.37-.78-.75-.55C9.29 3.71 6.68 8 8.87 13.62c.18.46-.36.89-.75.59-1.81-1.37-2-3.34-1.84-4.75.06-.52-.62-.77-.91-.34C4.69 10.16 4 11.84 4 14.37c.38 5.6 5.11 7.32 6.81 7.54 2.43.31 5.06-.14 6.95-1.87 2.08-1.93 2.84-5.01 1.72-7.69z'
GLYPH = {
 'flame': ('#FF8F1F', '<defs><linearGradient id="gf" x1="0" y1="1" x2="0" y2="0"><stop offset="0" stop-color="#FFC107"/><stop offset="1" stop-color="#FF3D00"/></linearGradient></defs>'
           f'<path fill="url(#gf)" d="{FLAME}"/><path fill="#FFF3B0" transform="translate(12 20.5) scale(.46) translate(-12 -20.5)" d="{FLAME}"/>'),
 'fan':   ('#4DD0E1', '<defs><linearGradient id="gb" x1="0" y1="0" x2="1" y2="1"><stop offset="0" stop-color="#7DF3FF"/><stop offset="1" stop-color="#2979FF"/></linearGradient></defs>'
           '<circle cx="12" cy="12" r="10.6" fill="none" stroke="#4DD0E1" stroke-opacity=".45" stroke-width="1.2"/>'
           '<g transform="translate(12 12)" fill="url(#gb)"><g id="b"><path d="M0-1.6C-3.6-3 -4.6-8.5 -1-10.6 2.2-11.8 3.6-8 2.2-4.6Z"/></g><use href="#b" transform="rotate(120)"/><use href="#b" transform="rotate(240)"/></g>'
           '<circle cx="12" cy="12" r="2.6" fill="#FFFFFF"/><circle cx="12" cy="12" r="1" fill="#2979FF"/>'),
 'drum':  ('#43D17A', '<path fill="none" stroke="#43D17A" stroke-width="2.4" stroke-linecap="round" d="M19.3 8.2A8.2 8.2 0 1 0 20.2 13"/><path fill="#43D17A" d="M14.6 7.4l6.6 1.2-.9-6.8z"/>'
           '<ellipse cx="12" cy="12.2" rx="2.9" ry="4.2" transform="rotate(35 12 12.2)" fill="#D79A5B"/><path stroke="#6B4423" stroke-width="1.1" fill="none" stroke-linecap="round" d="M10.4 14.6c1.8-1.3 2.7-3.5 3.2-5"/>'),
 'snow':  ('#A5B4FC', '<g fill="none" stroke="#C7D2FE" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path id="a" d="M12 2.5v19M9.2 5l2.8 2.6L14.8 5M9.2 19l2.8-2.6 2.8 2.6"/><use href="#a" transform="rotate(60 12 12)"/><use href="#a" transform="rotate(120 12 12)"/></g><circle cx="12" cy="12" r="2.2" fill="#FFFFFF"/>'),
}
def svg(name):
    col, g = GLYPH[name]
    return (f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 36 36" width="{PX*SC}" height="{PX*SC}">'
            f'<circle cx="18" cy="18" r="17" fill="{col}" fill-opacity="0.2" stroke="{col}" stroke-opacity="0.75" stroke-width="1.4"/>'
            f'<g transform="translate(5.5 5.5) scale(1.04)">{g}</g></svg>')

def render_svgs():
    from playwright.sync_api import sync_playwright
    out = {}
    with sync_playwright() as p:
        b = p.chromium.launch(); pg = b.new_page()
        for n in GLYPH:
            pg.set_content(f'<html><body style="margin:0;background:transparent">{svg(n)}</body></html>')
            png = pg.locator('svg').screenshot(omit_background=True)
            out[n] = Image.open(io.BytesIO(png)).convert('RGBA').resize((PX, PX), Image.LANCZOS)
        b.close()
    return out

def to_rgb565a8(im):
    rgb, a = bytearray(), bytearray()
    for (r, g, bl, al) in im.getdata():
        v = ((r >> 3) << 11) | ((g >> 2) << 5) | (bl >> 3)
        rgb += bytes((v & 0xFF, v >> 8)); a.append(al)
    return bytes(rgb + a)

def carr(name, data):
    lines = []
    for i in range(0, len(data), 24):
        lines.append('    ' + ','.join(f'0x{x:02X}' for x in data[i:i+24]) + ',')
    return f'static const uint8_t {name}[{len(data)}] __attribute__((aligned(4))) = {{\n' + '\n'.join(lines) + '\n};\n'

imgs = render_svgs()
imgs['hibean']  = Image.open(os.path.join(HERE, 'badge_hibean_136.png')).convert('RGBA').resize((PX, PX), Image.LANCZOS)
imgs['artisan'] = Image.open(os.path.join(HERE, 'badge_artisan_136.png')).convert('RGBA').resize((PX, PX), Image.LANCZOS)
# Bieu tuong Setting: tu anh mau tools/icon_settings_src.png (hinh xanh tren nen sang) -> mat na, to mau kem C_CREAM
SET_PX = int(re.search(r'#define\s+UI_SET_ICON\s+(\d+)', cfg).group(1))
src = Image.open(os.path.join(HERE, 'icon_settings_src.png')).convert('RGB')
bb_bg = src.getpixel((2, 2))[0]
mask = src.point(lambda v: 0).convert('L')
mask = Image.eval(src.split()[0], lambda r: max(0, min(255, int((bb_bg - r) * 255 / max(1, bb_bg - 50)))))
bbox = mask.getbbox(); mask = mask.crop(bbox)
side = int(max(mask.size) * 1.08); sq = Image.new('L', (side, side), 0); sq.paste(mask, ((side - mask.size[0]) // 2, (side - mask.size[1]) // 2))
alpha = sq.resize((SET_PX, SET_PX), Image.LANCZOS)
cream = Image.new('RGBA', (SET_PX, SET_PX), (0xF3, 0xE5, 0xD0, 255)); cream.putalpha(alpha)
imgs['settings'] = cream
os.makedirs(os.path.join(HERE, 'preview'), exist_ok=True)
for n, im in imgs.items(): im.save(os.path.join(HERE, 'preview', f'icon_{n}.png'))

out = ['// TU DONG SINH boi tools/make_icons.py - KHONG SUA TAY. Bieu tuong %d px, dinh dang RGB565A8 (mat RGB565 roi mat A8).' % PX,
       '#pragma once', '#include <stdint.h>', f'#define ICON_STD_PX {PX}', f'#define ICON_SET_PX {SET_PX}', '']
for n in ['flame', 'fan', 'drum', 'snow', 'hibean', 'artisan', 'settings']:
    out.append(carr(f'icon_{n}_std', to_rgb565a8(imgs[n])))
open(os.path.join(ROOT, 'lib', 'RoasterIcons_std.h'), 'w').write('\n'.join(out))
print('OK: lib/RoasterIcons_std.h', PX, 'px')
