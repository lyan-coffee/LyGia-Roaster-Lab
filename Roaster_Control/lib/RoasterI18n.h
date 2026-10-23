/*
 * GPL v3 - xem LyGia_Roaster.ino
 *
 * MODULE DA NGON NGU (English / Tieng Viet / Chinese)
 *
 * tr("KEY") tra ve chuoi theo ngon ngu hien tai; KEY chinh la chuoi tieng Anh.
 * Neu khong tim thay KEY thi tra lai chinh KEY.
 *
 * FONT: font Montserrat mac dinh cua LVGL chi co ASCII.
 *  - Chinese : bat trong lv_conf.h  LV_FONT_SOURCE_HAN_SANS_SC_16_CJK 1  (va _14_CJK 1),
 *              hoac dinh nghia I18N_FONT_ZH_14 / _16 / _20 bang font tu tao (lv_font_conv).
 *              Khong co font -> tu dong dung English.
 *  - Viet co dau : tao font bang lv_font_conv roi dinh nghia I18N_FONT_VI_14 / _16 / _20, vi du:
 *        LV_FONT_DECLARE(lv_font_vi_14) ... #define I18N_FONT_VI_14 (&lv_font_vi_14)
 *        lv_font_conv --font Montserrat-Regular.ttf --size 16 --bpp 4 --format lvgl
 *            -r 0x20-0x7F -r 0xB0 -r 0xC0-0x1B0 -r 0x1EA0-0x1EF9 --lv-font-name lv_font_vi_16 -o lv_font_vi_16.c
 *              Khong co font -> tu dong dung tieng Viet KHONG DAU.
 *  Phai goi i18n_init_fonts() mot lan truoc khi dung.
 */
#pragma once
#include <lvgl.h>
#include "RoasterSettings.h"

#if ENABLE_ZH_FONT
#include "RoasterFontZh.h"      // font tieng Hoa tu tao (du lieu o lib/lv_font_zh_*.h)
#endif

// ---- Chon font Chinese co san cua LVGL neu da bat trong lv_conf.h ----
#if !defined(I18N_FONT_ZH_14) && defined(LV_FONT_SOURCE_HAN_SANS_SC_14_CJK) && LV_FONT_SOURCE_HAN_SANS_SC_14_CJK
  #define I18N_FONT_ZH_14 (&lv_font_source_han_sans_sc_14_cjk)
#endif
#if !defined(I18N_FONT_ZH_16) && defined(LV_FONT_SOURCE_HAN_SANS_SC_16_CJK) && LV_FONT_SOURCE_HAN_SANS_SC_16_CJK
  #define I18N_FONT_ZH_16 (&lv_font_source_han_sans_sc_16_cjk)
#endif

struct I18nEntry {
    const char *en;
    const char *vi_ascii;   // tieng Viet khong dau (mac dinh)
    const char *vi;         // tieng Viet co dau (dung khi co font VI)
    const char *zh;
};

static const I18nEntry I18N[] = {
    // --- man hinh chinh ---   (GIU NGUYEN tieng Anh o moi ngon ngu: START, STOP, IDLE, ROASTING, COOLING, ON, OFF, No Signal, Connected, Wait, Off)
    {"Save",         "Luu",             "Lưu",            "保存"},
    {"Don't Save",   "Khong luu",       "Không lưu",      "不保存"},
    {"Soft",         "Em",              "Êm",             "柔和"},
    {"Balanced",     "Can bang",        "Cân bằng",       "平衡"},
    {"Fast",         "Nhanh",           "Nhanh",          "快速"},
    {"P on Error",   "P theo sai so",   "P theo sai số",  "P误差"},
    {"P on Meas",    "P theo do",       "P theo đo",      "P测量"},
    {"START",        "START",         "START",        "开始"},
    {"END",          "END",           "END",          "结束"},
    {"Manual",       "Thu Cong",      "Thủ Công",     "手动"},
    {"Auto",         "Tu Dong",       "Tự Động",      "自动"},
    {"Graph",        "Graph",           "Graph",          "曲线"},
    {"min",          "min",             "min",            "分"},
    {"m",            "m",               "m",              "分"},
    {"STOP",         "STOP",            "STOP",           "STOP"},
    {"IDLE",         "IDLE",             "IDLE",            "待机"},
    {"ROASTING",     "ROASTING",       "ROASTING",      "烘焙中"},
    {"COOLING",      "COOLING",    "COOLING",   "冷却中"},
    {"Bean Temp",    "BT",              "BT",             "BT"},
    {"Heater",       "Gia Nhiet",       "Gia Nhiệt",      "加热"},
    {"Air Fan",      "Quat Gio",        "Quạt Gió",       "风量"},
    {"Drum",         "Trong",           "Trống",          "滚筒"},
    {"Cool Fan",     "Quat Mat",        "Quạt Mát",       "冷却"},
    {"ON",           "ON",             "ON",            "开"},
    {"OFF",          "OFF",             "OFF",            "关"},
    {"No Signal",    "No Signal",    "No Signal",   "无信号"},
    {"Set",          "Dat",             "Đặt",            "设定"},
    {"Connected",    "Connected",          "Connected",         "已连接"},
    {"Wait",         "Wait",             "Wait",            "等待"},
    {"Off",          "Off",             "Off",            "关闭"},
    {"Disable HiBean?", "Tat ket noi HiBean?", "Tắt kết nối HiBean?", "关闭HiBean连接?"},
    {"Disable",      "Tat",             "Tắt",            "关闭"},
    {"Cancel",       "Huy",             "Hủy",            "取消"},
    {"Simulation",   "Mo Phong",        "Mô Phỏng",       "模拟"},
    {"Time",         "Thoi Gian",       "Thời Gian",      "时间"},
    {"Drop beans?",  "Xa hat?",         "Xả hạt?",        "确认下豆?"},
    {"End roast?",   "Ket thuc me rang?","Kết thúc mẻ rang?","结束烘焙?"},
    {"Confirm",      "Xac Nhan",        "Xác Nhận",       "确认"},
    // --- canh bao qua nhiet ---
    {"Overheat Alarm",         "Bao Qua Nhiet",     "Báo Quá Nhiệt",      "过热报警"},
    {"OVERHEAT",               "QUA NHIET",         "QUÁ NHIỆT",          "过热"},
    {"Limit",                  "Nguong",            "Ngưỡng",             "上限"},
    {"Reduce heater power now","Giam gia nhiet ngay","Giảm gia nhiệt ngay","请立即降低加热"},
    {"Heater Off",             "Tat Gia Nhiet",     "Tắt Gia Nhiệt",      "关闭加热"},
    {"Dismiss",                "Bo Qua",            "Bỏ Qua",             "忽略"},
    {"Test",                   "Thu",               "Thử",                "测试"},
    // --- canh bao (tu loi dieu khien) ---
    {"Press START",      "Nhan START",         "Nhấn START",           "请按开始"},
    {"Need DRUM ON",     "Can bat trong",      "Cần bật trống",        "需开滚筒"},
    {"Cool fan is ON",   "Quat mat dang bat",  "Quạt mát đang bật",    "冷却已开"},
    {AIR_LOW_MSG, "Can gio tu " CFG_STR(MIN_AIR_FOR_HEAT) "%", "Cần gió từ " CFG_STR(MIN_AIR_FOR_HEAT) "%", "风量需" CFG_STR(MIN_AIR_FOR_HEAT) "%以上"},
    {"HiBean lost",      "Mat HiBean",         "Mất HiBean",           "HiBean断开"},
    {"No temp signal",   "Mat tin hieu nhiet", "Mất tín hiệu nhiệt",   "无温度信号"},
    {"Press END",        "Nhan END",           "Nhấn END",             "请按END"},
    {"Press Charge",     "Nhan Charge",        "Nhấn Charge",          "请按Charge"},
    {"PID OFF",          "Tat PID",            "Tắt PID",              "PID已关"},
    {"E-STOP",           "DUNG KHAN CAP",      "DỪNG KHẨN CẤP",        "紧急停止"},
    // --- trang Setting ---
    {"SETTINGS",     "CAI DAT",         "CÀI ĐẶT",        "设置"},
    {"ORIENTATION",  "HUONG MAN HINH",  "HƯỚNG MÀN HÌNH", "屏幕方向"},
    {"Portrait",     "Doc",             "Dọc",            "竖屏"},
    {"Landscape",    "Ngang",           "Ngang",          "横屏"},
    {"TEMP UNIT",    "DON VI NHIET DO", "ĐƠN VỊ NHIỆT ĐỘ","温度单位"},
    {"LANGUAGE",     "NGON NGU",        "NGÔN NGỮ",       "语言"},
    {"Orientation",  "Huong Man Hinh",  "Hướng Màn Hình", "屏幕方向"},   // nhan the trang Setting
    {"Temp Unit",    "Don Vi Nhiet Do", "Đơn Vị Nhiệt Độ","温度单位"},
    {"Language",     "Ngon Ngu",        "Ngôn Ngữ",       "语言"},
    {"Idle Screen",  "Man Hinh Cho",    "Màn Hình Chờ",   "待机画面"},
    {"Start roast",    "Bat dau me rang", "Bắt đầu mẻ rang", "开始烘焙"},
    {"Start PreHeat", "Bat dau PreHeat", "Bắt đầu PreHeat", "开始预热"},
    {"Charge at",    "Charge tai",      "Charge tại",      "下豆温度"},
    {"Choose profile", "Chon profile",    "Chọn profile",    "选择曲线"},
    // --- trang Gioi thieu ---
    {"Phone",          "Dien thoai",   "Điện thoại",  "电话"},
    {"Address",        "Dia chi",      "Địa chỉ",     "地址"},
    {"Touch to close", "Cham de dong", "Chạm để đóng","轻触关闭"},
};
static const int I18N_COUNT = sizeof(I18N) / sizeof(I18N[0]);

// ---- Font theo ngon ngu: 3 co (0 = nho/14, 1 = vua/16, 2 = lon/20) ----
static lv_font_t g_font_zh[3], g_font_vi[3];
static bool g_zh_ok = false, g_vi_ok = false;

static const lv_font_t *i18n_pick(const lv_font_t *const cand[3], int c) {
    if (cand[c]) return cand[c];
    for (int d = 1; d < 3; d++) {                  // lay co gan nhat
        if (c - d >= 0 && cand[c - d]) return cand[c - d];
        if (c + d < 3  && cand[c + d]) return cand[c + d];
    }
    return nullptr;
}

// base[0..2] = font Montserrat tuong ung (lam fallback cho ky tu ASCII/bieu tuong)
static void i18n_init_fonts(const lv_font_t *b0, const lv_font_t *b1, const lv_font_t *b2) {
    const lv_font_t *base[3] = { b0, b1, b2 };
    const lv_font_t *zh[3] = {
#ifdef I18N_FONT_ZH_14
        I18N_FONT_ZH_14,
#else
        nullptr,
#endif
#ifdef I18N_FONT_ZH_16
        I18N_FONT_ZH_16,
#else
        nullptr,
#endif
#ifdef I18N_FONT_ZH_20
        I18N_FONT_ZH_20
#else
        nullptr
#endif
    };
    const lv_font_t *vi[3] = {
#ifdef I18N_FONT_VI_14
        I18N_FONT_VI_14,
#else
        nullptr,
#endif
#ifdef I18N_FONT_VI_16
        I18N_FONT_VI_16,
#else
        nullptr,
#endif
#ifdef I18N_FONT_VI_20
        I18N_FONT_VI_20
#else
        nullptr
#endif
    };
    g_zh_ok = g_vi_ok = false;
    for (int c = 0; c < 3; c++) {
        const lv_font_t *z = i18n_pick(zh, c);
        if (z) { g_font_zh[c] = *z; g_font_zh[c].fallback = base[c]; g_zh_ok = true;
                 g_font_zh[c].line_height = base[c]->line_height; g_font_zh[c].base_line = base[c]->base_line; }   // cung chieu cao dong voi font goc: khong lech/nhay hang
        const lv_font_t *v = i18n_pick(vi, c);
        if (v) { g_font_vi[c] = *v; g_font_vi[c].fallback = base[c]; g_vi_ok = true;
                 g_font_vi[c].line_height = base[c]->line_height; g_font_vi[c].base_line = base[c]->base_line; }
    }
}

static bool i18n_lang_usable(int lang) { return lang == LANG_ZH ? g_zh_ok : true; }

// Font cho mot ngon ngu bat ky (base: co nho/vua/lon theo thu tu 0/1/2)
static const lv_font_t *i18n_font_lang(int lang, int c, const lv_font_t *base) {
    if (lang == LANG_ZH && g_zh_ok) return &g_font_zh[c];
    if (lang == LANG_VI && g_vi_ok) return &g_font_vi[c];
    return base;
}

static const char *tr_lang(int lang, const char *en) {
    if (lang == LANG_EN) return en;
    for (int i = 0; i < I18N_COUNT; i++) {
        if (strcmp(I18N[i].en, en) == 0) {
            if (lang == LANG_VI) return g_vi_ok ? I18N[i].vi : I18N[i].vi_ascii;
            if (lang == LANG_ZH) return g_zh_ok ? I18N[i].zh : en;
        }
    }
    return en;
}

static const char *tr(const char *en) { return tr_lang(g_set.lang, en); }
