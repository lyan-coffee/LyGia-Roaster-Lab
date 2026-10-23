/*
 * GPL v3 - xem LyGia_Roaster.ino
 *
 * BIEU TUONG HUY HIEU (nhieu mau) cho the Heater / Air Fan / Drum / Cool Fan: nhung thang vao code, khong can file ngoai.
 *  - Mot co duy nhat ICON_STD_PX (34 px); dinh dang RGB565A8 (mau RGB565 + kenh alpha).
 *  - Drum / Cool khi TAT: cung anh nhung nhuom xam (icon_set_on(img, false)).
 *  - Du lieu anh o RoasterIcons_std.h, sinh bang tools/make_icons.py (khong sua tay).
 */
#pragma once

#include <lvgl.h>
#include "RoasterIcons_std.h"     // bieu tuong co chuan ICON_STD_PX (34 px, QuyTac_GiaoDien.txt Phan 3) - sinh bang tools/make_icons.py
enum { ICON_FLAME = 0, ICON_FAN, ICON_DRUM, ICON_SNOW, ICON_HIBEAN, ICON_ARTISAN, ICON_COUNT };
// ISZ_S / M / L: ten co cu, giu lai de code goi khong phai sua; tat ca deu tro ve anh 34 px nhu ISZ_STD
enum { ISZ_S = 0, ISZ_M, ISZ_L, ISZ_STD, ISZ_COUNT };
static const uint8_t ICON_PX[ISZ_COUNT] = { ICON_STD_PX, ICON_STD_PX, ICON_STD_PX, ICON_STD_PX };


// Bo cuc moi: moi co deu tro ve anh 34 px (ICON_PX ben duoi cung = ICON_STD_PX)
static const uint8_t *const ICON_DATA[ICON_COUNT][ISZ_COUNT] = {
    { icon_flame_std, icon_flame_std, icon_flame_std, icon_flame_std },
    { icon_fan_std, icon_fan_std, icon_fan_std, icon_fan_std },
    { icon_drum_std, icon_drum_std, icon_drum_std, icon_drum_std },
    { icon_snow_std, icon_snow_std, icon_snow_std, icon_snow_std },
    { icon_hibean_std, icon_hibean_std, icon_hibean_std, icon_hibean_std },
    { icon_artisan_std, icon_artisan_std, icon_artisan_std, icon_artisan_std },
};

// Tao anh bieu tuong (id: ICON_*, sz: ISZ_*) tai (x, y) trong parent
static lv_obj_t *make_icon(lv_obj_t *parent, int id, int sz, int x, int y) {
    static lv_image_dsc_t dsc[ICON_COUNT][ISZ_COUNT];
    static bool init = false;
    if (!init) {
        init = true;
        memset(dsc, 0, sizeof(dsc));
        for (int i = 0; i < ICON_COUNT; i++)
            for (int k = 0; k < ISZ_COUNT; k++) {
                lv_image_dsc_t &d = dsc[i][k];
                d.header.magic  = LV_IMAGE_HEADER_MAGIC;
                d.header.cf     = LV_COLOR_FORMAT_RGB565A8;
                d.header.w      = ICON_PX[k];
                d.header.h      = ICON_PX[k];
                d.header.stride = ICON_PX[k] * 2;
                d.data_size     = ICON_PX[k] * ICON_PX[k] * 3;
                d.data          = ICON_DATA[i][k];
            }
    }
    if (id >= ICON_HIBEAN) sz = ISZ_STD;                 // huy hieu HiBean / Artisan chi co co chuan
    lv_obj_t *img = lv_image_create(parent);
    lv_image_set_src(img, &dsc[id][sz]);
    lv_obj_set_pos(img, x, y);
    lv_obj_remove_flag(img, LV_OBJ_FLAG_CLICKABLE);
    return img;
}

// Huy hieu logo (HiBean / Artisan): tat = nga xam mot phan, mo di (van nhin ra logo)
static void icon_set_dim(lv_obj_t *img, bool on) {
    lv_obj_set_style_image_recolor(img, lv_color_hex(0x6E6258), 0);
    lv_obj_set_style_image_recolor_opa(img, on ? LV_OPA_TRANSP : LV_OPA_60, 0);
    lv_obj_set_style_image_opa(img, on ? LV_OPA_COVER : LV_OPA_60, 0);
}

// Bat = giu mau goc; tat = nhuom xam, mo di
static void icon_set_on(lv_obj_t *img, bool on) {
    lv_obj_set_style_image_recolor(img, lv_color_hex(0x9C8C7C), 0);
    lv_obj_set_style_image_recolor_opa(img, on ? LV_OPA_TRANSP : LV_OPA_COVER, 0);
    lv_obj_set_style_image_opa(img, on ? LV_OPA_COVER : LV_OPA_70, 0);
}
