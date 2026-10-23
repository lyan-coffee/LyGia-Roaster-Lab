/*
 * GPL v3 - xem LyGia_Roaster.ino
 *
 * FONT TIENG HOA TU TAO (bat bang ENABLE_ZH_FONT 1 trong LySetting.h).
 * Du lieu font nam ngay trong lib/: lv_font_zh_14.h, lv_font_zh_16.h, lv_font_zh_20.h (3 file nay chi duoc include o day).
 * Tao bang tools/make_zh_font.py (xem Readme.txt muc A-V11). Thieu 1 co thi dung co gan nhat; thieu ca 3 thi bo dong LV_FONT_DECLARE va #include tuong ung.
 */
#pragma once
#include <lvgl.h>
LV_FONT_DECLARE(lv_font_zh_14);
LV_FONT_DECLARE(lv_font_zh_16);
LV_FONT_DECLARE(lv_font_zh_20);
#include "lv_font_zh_14.h"
#include "lv_font_zh_16.h"
#include "lv_font_zh_20.h"
#define I18N_FONT_ZH_14 (&lv_font_zh_14)
#define I18N_FONT_ZH_16 (&lv_font_zh_16)
#define I18N_FONT_ZH_20 (&lv_font_zh_20)
