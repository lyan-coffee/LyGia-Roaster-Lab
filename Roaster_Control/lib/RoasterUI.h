/*
 * GPL v3 - xem LyGia_Roaster.ino
 *
 * Giao dien cam ung LVGL 9 cho Guition JC3248W535 (AXS15231B QSPI, panel goc 320x480).
 * - Toan bo LVGL chay trong MOT task (core 0); UI chi DOC anh chup trang thai (roasterSnapshot)
 *   va GOI cac ham panel*() cua loi dieu khien -> dong bo voi HiBean.
 * - Huong doc/ngang: xoay bang phan mem trong ham flush + anh xa lai toa do cam ung.
 * - Cai dat (huong, don vi C/F, ngon ngu): lib/RoasterSettings.h, trang Setting: RoasterSettingsUI.h
 *   (bat/tat bang ENABLE_SETTINGS_PAGE).
 */
#pragma once
#include <Wire.h>
#include <Arduino_GFX_Library.h>
#include <lvgl.h>
#include "SkiCMD.h"
#include "SkiBLE.h"
#include "UIConfig.h"           // THONG SO GIAO DIEN DUNG CHUNG (mau, co chu, kich thuoc, khoang cach) - xem QuyTac_GiaoDien.txt
#include "RoasterIcons.h"        // MODULE bieu tuong Heater/Air/Drum/Cool/HiBean/Artisan (34 px)

#if LV_COLOR_DEPTH != 16
#error "Hay dat LV_COLOR_DEPTH = 16 trong lv_conf.h"
#endif

// =============================================================================
// 1. PHAN CUNG
// =============================================================================
#define SCREEN_W 320          // kich thuoc VAT LY cua panel (doc)
#define SCREEN_H 480
#define TFT_BLK  1

#define TOUCH_ADDR 0x3B
#define TOUCH_SDA  4
#define TOUCH_SCL  8

#define TOUCH_SWAP_XY  0
#define TOUCH_INVERT_X 0
#define TOUCH_INVERT_Y 0

// Huong xoay khi che do NGANG: 0 hoac 1 (doi neu hinh bi lon nguoc 180 do)
#define LANDSCAPE_FLIP 0

Arduino_DataBus *bus   = new Arduino_ESP32QSPI(45 /*CS*/, 47 /*SCK*/, 21 /*D0*/, 48 /*D1*/, 40 /*D2*/, 39 /*D3*/);
Arduino_G *panel       = new Arduino_AXS15231B(bus, GFX_NOT_DEFINED, 0, false, SCREEN_W, SCREEN_H);
Arduino_Canvas *gfx    = new Arduino_Canvas(SCREEN_W, SCREEN_H, panel, 0, 0, 0);

// =============================================================================
// 2. CAU HINH GIAO DIEN
// =============================================================================
#define PID_STEP_C        5            // moi lan bam +/- doi muc nhiet Set 5 do C
#define STEP_HEATER       5
#define STEP_AIR          5
#define TEMP_MAX          300           // thang do thanh nhiet do (do C)
#define TEMP_COLOR_FROM   30
#define TEMP_COLOR_TO     230

static const uint32_t C_BG     = 0x14100D;
static const uint32_t C_CARD   = 0x221B16;
static const uint32_t C_BORDER = 0x3A2E26;
static const uint32_t C_TRACK  = 0x3A2E26;
static const uint32_t C_BTN    = 0x33281F;
static const uint32_t C_CREAM  = 0xF3E5D0;
static const uint32_t C_MUTED  = 0x9C8C7C;
static const uint32_t C_ORANGE = 0xFF8F1F;
static const uint32_t C_GREEN  = 0x43D17A;
static const uint32_t C_BLUE   = 0x38BDF8;
static const uint32_t C_RED    = 0xFF5252;
static const uint32_t C_HIBEAN = 0xFFB300;

#if LV_FONT_MONTSERRAT_48
#define F_HUGE (&lv_font_montserrat_48)
#else
#define F_HUGE (&lv_font_montserrat_14)
#endif
#if LV_FONT_MONTSERRAT_28
#define F_BIG (&lv_font_montserrat_28)
#else
#define F_BIG (&lv_font_montserrat_14)
#endif
#if LV_FONT_MONTSERRAT_20
#define F_MED (&lv_font_montserrat_20)
#else
#define F_MED (&lv_font_montserrat_14)
#endif
#if LV_FONT_MONTSERRAT_16
#define F_NORM (&lv_font_montserrat_16)
#else
#define F_NORM (&lv_font_montserrat_14)
#endif
#define F_SMALL (&lv_font_montserrat_14)

#include "RoasterSettings.h"   // g_set (huong / don vi / ngon ngu)
#include "RoasterI18n.h"       // tr(), font theo ngon ngu
#include "RoasterAlarmSound.h" // loa canh bao qua nhiet (task rieng)

// Font cho van ban co the dich: chon theo ngon ngu hien tai
static const lv_font_t *fnt(const lv_font_t *base) {
    int c = (base == F_SMALL) ? 0 : (base == F_NORM) ? 1 : (base == F_MED) ? 2 : -1;
    if (c < 0) return base;
    return i18n_font_lang(g_set.lang, c, base);
}

// =============================================================================
// 3. TRANG THAI GIAO DIEN
// =============================================================================
enum { PAGE_MAIN = 0, PAGE_SETTINGS = 1 };

static lv_display_t *g_disp = nullptr;
static int  g_page = PAGE_MAIN;
static bool g_main_built = false;          // cac widget trang chinh con hop le?
static uint8_t g_applied_orient = 0;       // huong dang ap dung cho man hinh/LVGL

static inline bool ui_landscape() { return g_applied_orient == ORI_LANDSCAPE; }
static inline int ui_w() { return ui_landscape() ? SCREEN_H : SCREEN_W; }
static inline int ui_h() { return ui_landscape() ? SCREEN_W : SCREEN_H; }

static float to_disp(float c)         { return g_set.unit == UNIT_F ? c * 1.8f + 32.0f : c; }
static float from_disp(float d)       { return g_set.unit == UNIT_F ? (d - 32.0f) / 1.8f : d; }
static const char *unit_txt()         { return g_set.unit == UNIT_F ? "°F" : "°C"; }

static int g_hint_right = 100;                 // mep trai nut dau tien ben phai header (gioi han vung canh bao)
static lv_obj_t *lb_temp, *lb_stage, *lb_bt_title;
static lv_obj_t *lb_status, *btn_start, *lb_start, *lb_timer, *lb_ror;
static lv_obj_t *card_hibean, *logo_hibean, *lb_link;
static lv_obj_t *ev_btn[EV_COUNT], *ev_name[EV_COUNT], *ev_time[EV_COUNT];   // cum nut moc me rang
static lv_obj_t *sw_drum, *lb_drum, *sw_cool, *lb_cool;
// Bo cuc V3: hang dieu khien 1 dong dung chung trang chinh va trang bieu do
struct CtlRow { lv_obj_t *icon, *pidlb, *slider, *value, *sv; };
static CtlRow m_heat = {}, m_air = {};          // hang Heater / Air trang chinh (bo cuc moi)
static void v3_ctl_apply(CtlRow &r, const Snap &s, bool heater, int &c_pid);   // dinh nghia o muc 6b
static int  c_m_pid = -1;
static lv_obj_t *ic_drum = nullptr, *ic_cool = nullptr;   // bieu tuong Drum / Cool (nhuom xam khi tat)
static int c_ic_drum = -1, c_ic_cool = -1;

// bo nho dem "gia tri cu" (reset moi lan dung lai giao dien)
static int c_last_key = -99, c_last_mode = -1, c_last_tstate = -1;
static int c_ev_state[EV_COUNT] = {-1, -1, -1, -1, -1, -1}, c_last_ror = -1;
static int c_ev_en[EV_COUNT]    = {-1, -1, -1, -1, -1, -1};   // nut mo/khoa (chua START)
static int c_ev_tm[EV_COUNT]    = {-1, -1, -1, -1, -1, -1};   // co dong gio hay khong (de can giua nhan)
static int ev_bh = 34;                                         // chieu cao nut moc
static const char *EV_NAMES[EV_COUNT] = { "PR", "CH", "DE", "FCs", "SCs", "DR" };   // viet tat chuan quoc te (QuyTac_GiaoDien.txt)   // ten moc giu nguyen tieng Anh o moi ngon ngu

static void ui_request_page(int page);     // dinh nghia o duoi
static void alarm_tick(const Snap &s);     // RoasterAlarm.h
static void idle_tick(const Snap &s);      // RoasterIdle.h
static void ui_confirm_timed(int kind);
#if (ENABLE_START_DIALOG && ENABLE_PROFILE)
static void ui_start_choose();             // RoasterStartMode.h
#endif    // hop thoai Drop (1) / END (2) co dem nguoc 5 giay (dinh nghia o duoi)
static void confirm_timed_tick(const Snap &s);
#if ENABLE_ROAST_GRAPH
static void graph_tick(const Snap &s);     // RoasterGraph.h
#endif

// =============================================================================
// 4. CAP NHAT GIAO DIEN TU TRANG THAI
// =============================================================================
static void set_text_if_changed(lv_obj_t *lbl, const char *txt) {
    if (strcmp(lv_label_get_text(lbl), txt) != 0) lv_label_set_text(lbl, txt);
}

static void set_switch(lv_obj_t *sw, bool on) {
    if (on) lv_obj_add_state(sw, LV_STATE_CHECKED); else lv_obj_remove_state(sw, LV_STATE_CHECKED);
}

// Can giua nhan trong nut theo chieu cao: co gio -> ten + gio xep doc o giua; khong co gio -> ten o giua
static void ev_layout(int i, bool has_time) {
    const int lh = (int)lv_font_get_line_height(lv_obj_get_style_text_font(ev_name[i], LV_PART_MAIN));
    if (has_time) {
        const int y0 = (ev_bh - 2 * lh) / 2;
        lv_obj_set_y(ev_name[i], y0);
        lv_obj_set_y(ev_time[i], y0 + lh);
        lv_obj_remove_flag(ev_time[i], LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_set_y(ev_name[i], (ev_bh - lh) / 2);
        lv_obj_add_flag(ev_time[i], LV_OBJ_FLAG_HIDDEN);
    }
}

// Cum nut moc: 0 = da bam (xanh), 1 = moc ke tiep (cam), 2 = chua toi luot (mo), 3 = Drop dang cho xac nhan (do), 4 = Drop san sang (vien do)
static void ui_apply_events(const Snap &s) {
    int last = -1;
    for (int i = 0; i < EV_COUNT; i++) if (s.evDone & (1 << i)) last = i;
    const int next = last + 1;
    char buf[24];
    for (int i = 0; i < EV_COUNT; i++) {
        const bool done = (s.evDone & (1 << i)) != 0;
        const bool armed = false;                                       // xac nhan Drop bang hop thoai dem nguoc (ui_confirm_timed), nut khong doi
        // Drop luon "san sang" (vien do) vi bam duoc bat cu luc nao; cac moc khac mo di cho den luot
        const int st = armed ? 3 : (done ? 0 : (i == next ? 1 : (i == EV_DROP ? 4 : 2)));

        set_text_if_changed(ev_name[i], armed ? (lv_obj_get_width(ev_btn[i]) < 70 ? "DR?" : "Confirm") : EV_NAMES[i]);   // ten nut moc la thuat ngu quoc te: khong dich
        if (armed)                          snprintf(buf, sizeof(buf), "%us", (unsigned)((s.dropArmMs + 999) / 1000));
        else if (done && i != EV_PREHEAT && s.evMs[i] != EV_NO_TIME)
        {
            snprintf(buf, sizeof(buf), "%u:%02u", (unsigned)(s.evMs[i] / 60000), (unsigned)((s.evMs[i] / 1000) % 60));
        }
        else                                buf[0] = 0;
        set_text_if_changed(ev_time[i], buf);
        const int has_t = buf[0] ? 1 : 0;
        if (has_t != c_ev_tm[i]) { c_ev_tm[i] = has_t; ev_layout(i, has_t); }

        // Chi bam duoc khi da START: chua START thi nut mo va khoa cam ung
        const int en = (s.roasting ? 1 : 0) | (done ? 2 : 0);          // bit0: dang START, bit1: moc da bam
        if (en != c_ev_en[i]) {
            c_ev_en[i] = en;
            if (s.roasting) lv_obj_remove_state(ev_btn[i], LV_STATE_DISABLED);
            else            lv_obj_add_state(ev_btn[i], LV_STATE_DISABLED);
            lv_obj_set_style_opa(ev_btn[i], (s.roasting || done) ? LV_OPA_COVER : LV_OPA_50, 0);   // moc da bam van ro de xem gio
        }

        if (st != c_ev_state[i]) {
            c_ev_state[i] = st;
            uint32_t bg, bd, tc;
            switch (st) {
                case 0:  bg = C_BTN;    bd = 0x2E6A47; tc = C_GREEN;  break;
                case 1:  bg = C_ORANGE; bd = C_ORANGE; tc = C_BG;     break;
                case 3:  bg = C_RED;    bd = C_RED;    tc = 0xFFFFFF; break;
                case 4:  bg = C_CARD;   bd = 0x8E2B2B; tc = C_CREAM;  break;
                default: bg = C_CARD;   bd = C_BORDER; tc = C_MUTED;  break;
            }
            lv_obj_set_style_bg_color(ev_btn[i], lv_color_hex(bg), 0);
            lv_obj_set_style_border_color(ev_btn[i], lv_color_hex(bd), 0);
            lv_obj_set_style_text_color(ev_name[i], lv_color_hex(tc), 0);
            lv_obj_set_style_text_color(ev_time[i], lv_color_hex(tc), 0);
        }
    }
}

// Nut START / END (dung chung trang chinh va trang bieu do):
//  - chua co me rang: "START" (xanh)  - me rang dang mo: "END" (do)  - dang cho xac nhan END: "Confirm Ns" (cam)
static int c_start_key = -1;             // bo nho dem; reset khi dung lai giao dien
static void apply_start_button(lv_obj_t *btn, lv_obj_t *lb, const Snap &s) {
    const bool armed = false;                                       // xac nhan END bang hop thoai dem nguoc (ui_confirm_timed), nut khong doi
    const int key = (s.live ? 1 : 0) | (armed ? (2 | (int)(((s.endArmMs + 999) / 1000) << 2)) : 0);
    if (key == c_start_key) return;
    c_start_key = key;
    char buf[24];
    if (!s.live)    snprintf(buf, sizeof(buf), "%s", tr("START"));
    else if (armed) snprintf(buf, sizeof(buf), "Confirm %us", (unsigned)((s.endArmMs + 999) / 1000));
    else            snprintf(buf, sizeof(buf), "%s", tr("END"));
    set_text_if_changed(lb, buf);
    lv_obj_set_style_text_font(lb, armed ? fnt(F_NORM) : fnt(F_MED), 0);
    lv_obj_center(lb);
    lv_obj_set_style_bg_color(btn, lv_color_hex(!s.live ? C_GREEN : (armed ? C_ORANGE : C_RED)), 0);
}

// RoR co duoc hien khong: da Charge, co so lieu, >= ROR_MIN_SHOW (co tre ROR_HYST de khong chop quanh nguong)
static bool ror_visible(const Snap &s) {
    static bool shown = false;
    if (!(s.rorOk && s.tempOk && s.charged)) { shown = false; return false; }
    shown = shown ? (s.ror >= ROR_MIN_SHOW - ROR_HYST) : (s.ror >= ROR_MIN_SHOW);
    return shown;
}

static void ui_apply(const Snap &s) {
    char buf[48];

    // ---- Nhiet do hat (tinh bang do C, hien thi theo don vi da chon) ----
    snprintf(buf, sizeof(buf), "%s%s", "BT", s.sim ? "  [SIM]" : "");
    set_text_if_changed(lb_bt_title, buf);
    lv_obj_set_style_text_color(lb_bt_title, lv_color_hex(s.sim ? C_ORANGE : C_MUTED), 0);
    float t = constrain(s.temp, 0.0f, (float)TEMP_MAX);
    int mode = s.hint[0] ? 1 : (s.tempOk ? 0 : 2);      // 1 = canh bao, 2 = mat tin hieu
    float k = constrain((t - TEMP_COLOR_FROM) / (float)(TEMP_COLOR_TO - TEMP_COLOR_FROM), 0.0f, 1.0f);
    int hue = (int)(215.0f * (1.0f - k));

    if (s.tempOk) snprintf(buf, sizeof(buf), "%.1f%s", to_disp(t), unit_txt());
    else          snprintf(buf, sizeof(buf), "--.-%s", unit_txt());
    set_text_if_changed(lb_temp, buf);

#if ENABLE_OVERHEAT_ALARM
    const bool over = s.tempOk && s.temp >= (float)g_set.warn;       // qua muc canh bao: so nhiet do chuyen do
#else
    const bool over = false;
#endif
    int key = (mode == 2) ? -1 : (over ? -2 : hue);
    if (key != c_last_key || mode != c_last_mode) {
        c_last_key = key; c_last_mode = mode;
        lv_color_t col = (mode == 2) ? lv_color_hex(C_MUTED)
                       : (over ? lv_color_hex(C_RED) : lv_color_hsv_to_rgb(hue, 95, 100));
        lv_obj_set_style_text_color(lb_temp, col, 0);
    }
    // Dong nho trong the nhiet do: chi hien canh bao tu loi dieu khien (vi du "Press START"), khong hien giai doan rang
    set_text_if_changed(lb_stage, mode == 1 ? tr(s.hint) : "");
    if (mode == 1) {                                   // canh bao nam tren dong IDLE, canh giua khoang trong con lai
        lv_obj_update_layout(lb_status);
        int a = lv_obj_get_x(lb_status) + lv_obj_get_width(lb_status) + 8;
        int b = g_hint_right - 8;
        if (b - a < 40) b = a + 40;
        lv_obj_set_width(lb_stage, b - a);
        lv_obj_set_pos(lb_stage, a, lv_obj_get_y(lb_status));
    }

    // ---- Time (tinh tu Charge) + RoR ----
    uint32_t sec = s.batchMs / 1000;
    snprintf(buf, sizeof(buf), "%02u:%02u", (unsigned)(sec / 60), (unsigned)(sec % 60));
    set_text_if_changed(lb_timer, buf);
    int tstate = s.batchRun ? 1 : (s.charged ? 2 : 0);
    if (tstate != c_last_tstate) {
        c_last_tstate = tstate;
        uint32_t col = (tstate == 1) ? C_CREAM : (tstate == 2 ? C_GREEN : C_MUTED);
        lv_obj_set_style_text_color(lb_timer, lv_color_hex(col), 0);
    }
    const bool rok = ror_visible(s);       // RoR chi hien tu luc Charge va khi >= ROR_MIN_SHOW
    if (rok) snprintf(buf, sizeof(buf), "%.1f", g_set.unit == UNIT_F ? s.ror * 1.8f : s.ror);
    else     snprintf(buf, sizeof(buf), "--");
    set_text_if_changed(lb_ror, buf);
    if ((int)rok != c_last_ror) {
        c_last_ror = rok;
        lv_obj_set_style_text_color(lb_ror, lv_color_hex(rok ? C_BLUE : C_MUTED), 0);
    }
#if !ENABLE_ROAST_GRAPH
    ui_apply_events(s);                  // (co module bieu do: cum nut moc nam o trang bieu do, cap nhat trong graph_tick)
#endif

    v3_ctl_apply(m_heat, s, true, c_m_pid);
    v3_ctl_apply(m_air, s, false, c_m_pid);

    // ---- Drum / Cool ----
    bool drum = s.drum > 0, cool = s.cool > 0;
    set_switch(sw_drum, drum);
    set_text_if_changed(lb_drum, tr(drum ? "ON" : "OFF"));
    lv_obj_set_style_text_color(lb_drum, lv_color_hex(drum ? C_GREEN : C_MUTED), 0);
    set_switch(sw_cool, cool);
    if (ic_drum && c_ic_drum != (int)drum) { c_ic_drum = drum; icon_set_on(ic_drum, drum); }
    if (ic_cool && c_ic_cool != (int)cool) { c_ic_cool = cool; icon_set_on(ic_cool, cool); }
    set_text_if_changed(lb_cool, tr(cool ? "ON" : "OFF"));
    lv_obj_set_style_text_color(lb_cool, lv_color_hex(cool ? C_BLUE : C_MUTED), 0);

    // ---- Dong duoi ten may: "No Signal" khi chua co tin hieu may rang, co tin hieu moi hien trang thai ----
    if (!s.tempOk) {
        set_text_if_changed(lb_status, tr("No Signal"));
        lv_obj_set_style_text_color(lb_status, lv_color_hex(C_RED), 0);
    } else {
        const char *st = "IDLE";
        uint32_t scol = C_MUTED;
        if (cool)           { st = "COOLING";  scol = C_BLUE; }
        else if (s.roasting){ st = "ROASTING"; scol = C_ORANGE; }
        if (s.pid) snprintf(buf, sizeof(buf), "%s | PID %d%s", tr(st), (int)lroundf(to_disp(s.sp)), unit_txt());
        else       snprintf(buf, sizeof(buf), "%s", tr(st));
        set_text_if_changed(lb_status, buf);
        lv_obj_set_style_text_color(lb_status, lv_color_hex(scol), 0);
    }

    apply_start_button(btn_start, lb_start, s);

    // ---- HiBean: 0 = tat, 1 = cho ket noi, 2 = da ket noi ----
    int link = hibeanLinkState();
    if (card_hibean) {                              // huy hieu + chu trang thai, vien theo trang thai
        set_text_if_changed(lb_link, tr(link == 2 ? "Connected" : (link == 1 ? "Wait" : "Off")));
        const uint32_t lc = link == 2 ? UI_C_OK : (link == 1 ? UI_C_HIBEAN : UI_C_MUTED);
        lv_obj_set_style_text_color(lb_link, lv_color_hex(lc), 0);
        lv_obj_set_style_border_color(card_hibean, lv_color_hex(link > 0 ? lc : UI_C_BORDER), 0);
        icon_set_dim(logo_hibean, link > 0);
    }
}

static void ui_poll() {
    if (!g_main_built) return;           // dang o trang Setting / dang dung lai giao dien
    Snap s;
    roasterSnapshot(s);
    ui_apply(s);
}

// 100 ms/lan: keo thay doi tu HiBean + kiem tra qua nhiet (chay o moi trang)
static void sync_cb(lv_timer_t *) {
    Snap s;
    roasterSnapshot(s);
    alarm_tick(s);
    idle_tick(s);
    confirm_timed_tick(s);
    if (g_main_built) ui_apply(s);
#if ENABLE_ROAST_GRAPH
    graph_tick(s);
#endif
}

// =============================================================================
// 5. SU KIEN TRANG CHINH (goi thang vao loi dieu khien)
// =============================================================================
// Bam +/-: luon nhay ve boi so cua buoc (5%), ke ca khi gia tri hien tai le (keo thanh truot / HiBean dat 7%, 12%...)
static int snap_step(int cur, int d) {
    const int st = d < 0 ? -d : d;
    if (st <= 0) return cur;
    int r = cur % st;
    if (d > 0) return cur - r + st;
    return r ? cur - r : cur - st;
}
static void ev_heater_step(lv_event_t *e) {
    Snap s; roasterSnapshot(s);
    int d = (int)(intptr_t)lv_event_get_user_data(e);
    if (s.pid) {                                   // PID: +/- doi muc nhiet Set 5 do C moi lan
        int cur = (int)lroundf(s.sp / (float)PID_STEP_C) * PID_STEP_C;
        panelSetSetpoint((float)(cur + (d > 0 ? PID_STEP_C : -PID_STEP_C)));
    } else {                                       // Manual: +/- doi cong suat (khong lap khi giu)
        if (lv_event_get_code(e) == LV_EVENT_LONG_PRESSED_REPEAT) return;
        panelSetHeater(snap_step(s.heat, d));
    }
    ui_poll();
}
static void ev_pid_toggle(lv_event_t *) {
    Snap s; roasterSnapshot(s);
    panelSetPID(!s.pid);
    ui_poll();
}
static void ev_air_step(lv_event_t *e) {
    Snap s; roasterSnapshot(s);
    panelSetAir(snap_step(s.vent, (int)(intptr_t)lv_event_get_user_data(e)));
    ui_poll();
}
static void ev_heater_slider(lv_event_t *e) { panelSetHeater(lv_slider_get_value(lv_event_get_target_obj(e))); ui_poll(); }
static void ev_air_slider(lv_event_t *e)    { panelSetAir(lv_slider_get_value(lv_event_get_target_obj(e)));    ui_poll(); }
static void ev_drum_sw(lv_event_t *e) { panelSetDrum(lv_obj_has_state(lv_event_get_target_obj(e), LV_STATE_CHECKED)); ui_poll(); }
static void ev_cool_sw(lv_event_t *e) { panelSetCool(lv_obj_has_state(lv_event_get_target_obj(e), LV_STATE_CHECKED)); ui_poll(); }
// (Bat/tat che do gia lap: CHI trong trang Setting)
static void ev_start(lv_event_t *) {
    Snap s; roasterSnapshot(s);
    if (s.live) {
        panelEnd();
        roasterSnapshot(s);
        if (s.live && s.endArmMs > 0) ui_confirm_timed(2);       // chua Drop: hoi lai, dem nguoc 5 giay
    } else {
#if (ENABLE_START_DIALOG && ENABLE_PROFILE)
        ui_start_choose();                                       // chon Manual / Auto
#else
        panelStart();
#endif
    }
    ui_poll();
}
static void ev_event_btn(lv_event_t *e) {
    int i = (int)(intptr_t)lv_event_get_user_data(e);
    if (i == EV_DROP) {
        panelDrop();
        Snap s; roasterSnapshot(s);
        if (s.dropArmMs > 0) ui_confirm_timed(1);                // hoi lai, dem nguoc 5 giay (het gio tu huy)
    } else panelEvent(i);
    ui_poll();
}
static void ev_card_tap(lv_event_t *e) {
    lv_obj_t *sw = (lv_obj_t *)lv_event_get_user_data(e);
    if (lv_obj_has_state(sw, LV_STATE_CHECKED)) lv_obj_remove_state(sw, LV_STATE_CHECKED);
    else lv_obj_add_state(sw, LV_STATE_CHECKED);
    lv_obj_send_event(sw, LV_EVENT_VALUE_CHANGED, NULL);
}

// =============================================================================
// 6. THANH PHAN GIAO DIEN DUNG CHUNG
// =============================================================================
static lv_obj_t *make_label(lv_obj_t *parent, const char *txt, const lv_font_t *font, uint32_t color) {
    lv_obj_t *l = lv_label_create(parent);
    lv_label_set_text(l, txt);
    lv_obj_set_style_text_font(l, font, 0);
    lv_obj_set_style_text_color(l, lv_color_hex(color), 0);
    return l;
}

static lv_obj_t *g_card_parent = nullptr;     // != nullptr: the dat vao vung cuon nay (trang Setting) thay vi man hinh
static lv_obj_t *make_card(int x, int y, int w, int h) {
    lv_obj_t *c = lv_obj_create(g_card_parent ? g_card_parent : lv_screen_active());
    lv_obj_remove_style_all(c);
    lv_obj_set_pos(c, x, y);
    lv_obj_set_size(c, w, h);
    lv_obj_set_style_bg_color(c, lv_color_hex(C_CARD), 0);
    lv_obj_set_style_bg_opa(c, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(c, 16, 0);
    lv_obj_set_style_border_width(c, 1, 0);
    lv_obj_set_style_border_color(c, lv_color_hex(C_BORDER), 0);
    lv_obj_remove_flag(c, LV_OBJ_FLAG_SCROLLABLE);
    return c;
}

static void make_step_button(lv_obj_t *parent, const char *symbol, int x, int y, int w,
                             lv_event_cb_t cb, int delta, bool repeat, int h = 40, int radius = 12) {
    lv_obj_t *b = lv_button_create(parent);
    lv_obj_remove_style_all(b);
    lv_obj_set_pos(b, x, y);
    lv_obj_set_size(b, w, h);
    lv_obj_set_style_bg_color(b, lv_color_hex(C_BTN), 0);
    lv_obj_set_style_bg_opa(b, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(b, radius, 0);
    lv_obj_set_style_border_width(b, 1, 0);
    lv_obj_set_style_border_color(b, lv_color_hex(C_BORDER), 0);
    lv_obj_set_style_bg_color(b, lv_color_hex(C_ORANGE), LV_STATE_PRESSED);
    lv_obj_t *l = make_label(b, symbol, F_MED, C_CREAM);
    lv_obj_center(l);
    lv_obj_add_event_cb(b, cb, LV_EVENT_SHORT_CLICKED, (void *)(intptr_t)delta);
    if (repeat) lv_obj_add_event_cb(b, cb, LV_EVENT_LONG_PRESSED_REPEAT, (void *)(intptr_t)delta);
}

#include "RoasterAlarm.h"        // MODULE man hinh canh bao qua nhiet (tat bang ENABLE_OVERHEAT_ALARM 0)
#include "RoasterIdle.h"         // MODULE man hinh cho (logo + dai sang) (tat bang ENABLE_IDLE_SCREEN 0)
#if ENABLE_SETTINGS_PAGE
#include "RoasterSettingsUI.h"   // MODULE trang Setting (tat bang ENABLE_SETTINGS_PAGE 0)
#endif

static lv_obj_t *make_slider(lv_obj_t *parent, int x, int y, int w, uint32_t c1, uint32_t c2, lv_event_cb_t cb) {
    lv_obj_t *s = lv_slider_create(parent);
    lv_obj_remove_style_all(s);
    lv_obj_set_pos(s, x, y);
    lv_obj_set_size(s, w, 14);
    lv_slider_set_range(s, 0, 100);

    lv_obj_set_style_bg_color(s, lv_color_hex(C_TRACK), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(s, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_radius(s, LV_RADIUS_CIRCLE, LV_PART_MAIN);

    lv_obj_set_style_bg_color(s, lv_color_hex(c1), LV_PART_INDICATOR);
    lv_obj_set_style_bg_grad_color(s, lv_color_hex(c2), LV_PART_INDICATOR);
    lv_obj_set_style_bg_grad_dir(s, LV_GRAD_DIR_HOR, LV_PART_INDICATOR);
    lv_obj_set_style_bg_opa(s, LV_OPA_COVER, LV_PART_INDICATOR);
    lv_obj_set_style_radius(s, LV_RADIUS_CIRCLE, LV_PART_INDICATOR);

    lv_obj_set_style_bg_color(s, lv_color_hex(C_CREAM), LV_PART_KNOB);
    lv_obj_set_style_bg_opa(s, LV_OPA_COVER, LV_PART_KNOB);
    lv_obj_set_style_radius(s, LV_RADIUS_CIRCLE, LV_PART_KNOB);
    lv_obj_set_style_pad_all(s, 4, LV_PART_KNOB);
    lv_obj_set_style_border_width(s, 2, LV_PART_KNOB);
    lv_obj_set_style_border_color(s, lv_color_hex(c2), LV_PART_KNOB);

    lv_obj_add_event_cb(s, cb, LV_EVENT_VALUE_CHANGED, NULL);
    return s;
}

// Cum 6 nut moc: PreHeat, Charge, DE / 1FC, 2FC, Drop (mac dinh 2 hang x 3 cot; cols = 2 -> 3 hang x 2 cot)
static void build_event_cluster(int x, int y, int w, int h, int cols = 3, int gap = 5, int radius = 12) {
    const int rows = EV_COUNT / cols;
    const int bw = (w - (cols - 1) * gap) / cols;
    const int bh = (h - (rows - 1) * gap) / rows;
    for (int i = 0; i < EV_COUNT; i++) {
        const int bx = x + (i % cols) * (bw + gap);
        const int by = y + (i / cols) * (bh + gap);
        lv_obj_t *b = lv_button_create(lv_screen_active());
        lv_obj_remove_style_all(b);
        lv_obj_set_pos(b, bx, by);
        lv_obj_set_size(b, bw, bh);
        lv_obj_set_style_bg_color(b, lv_color_hex(C_CARD), 0);
        lv_obj_set_style_bg_opa(b, LV_OPA_COVER, 0);
        lv_obj_set_style_radius(b, radius, 0);
        lv_obj_set_style_border_width(b, 1, 0);
        lv_obj_set_style_border_color(b, lv_color_hex(C_BORDER), 0);
        lv_obj_set_style_opa(b, LV_OPA_70, LV_STATE_PRESSED);
        lv_obj_remove_flag(b, LV_OBJ_FLAG_SCROLLABLE);

        ev_name[i] = make_label(b, EV_NAMES[i], fnt(F_SMALL), C_MUTED);
        lv_obj_set_width(ev_name[i], bw - 4);
        lv_label_set_long_mode(ev_name[i], LV_LABEL_LONG_CLIP);   // nut hep (cot 2 nut o trang bieu do ngang): khong xuong dong
        lv_obj_set_style_text_align(ev_name[i], LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_x(ev_name[i], 0);
        ev_time[i] = make_label(b, "", F_SMALL, C_MUTED);
        lv_obj_set_width(ev_time[i], bw - 4);
        lv_obj_set_style_text_align(ev_time[i], LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_x(ev_time[i], 0);
        ev_bh = bh;
        ev_layout(i, false);

        ev_btn[i] = b;
        c_ev_state[i] = -1; c_ev_en[i] = -1; c_ev_tm[i] = -1;
        lv_obj_add_event_cb(b, ev_event_btn, LV_EVENT_CLICKED, (void *)(intptr_t)i);
    }
}


// =============================================================================
// 6b. THANH PHAN BO CUC MOI (QuyTac_GiaoDien.txt, so lieu trong lib/UIConfig.h)
// =============================================================================
static lv_obj_t *v3_card(int x, int y, int w, int h) {
    lv_obj_t *c = make_card(x, y, w, h);
    lv_obj_set_style_radius(c, UI_RADIUS, 0);
    return c;
}
// Nut tieu de / nut thuong: nen, vien, chu
static lv_obj_t *v3_button(lv_obj_t *parent, int x, int y, int w, int h, uint32_t bg, uint32_t border,
                           const char *txt, const lv_font_t *f, uint32_t tc, lv_obj_t **out_lb = nullptr) {
    lv_obj_t *b = lv_button_create(parent);
    lv_obj_remove_style_all(b);
    lv_obj_set_pos(b, x, y);
    lv_obj_set_size(b, w, h);
    lv_obj_set_style_bg_color(b, lv_color_hex(bg), 0);
    lv_obj_set_style_bg_opa(b, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(b, UI_RADIUS, 0);
    if (border != bg) { lv_obj_set_style_border_width(b, 1, 0); lv_obj_set_style_border_color(b, lv_color_hex(border), 0); }
    lv_obj_set_style_opa(b, LV_OPA_70, LV_STATE_PRESSED);
    lv_obj_t *l = make_label(b, txt, f, tc);
    lv_obj_center(l);
    if (out_lb) *out_lb = l;
    return b;
}

// Duong ke tieu de (bo cuc moi): thang le trai / phai, o y = UI_HEADER_H
static void v3_header_line(int W) {
    lv_obj_t *line = lv_obj_create(lv_screen_active());
    lv_obj_remove_style_all(line);
    lv_obj_set_pos(line, UI_MARGIN, UI_HEADER_H);
    lv_obj_set_size(line, W - 2 * UI_MARGIN, 2);
    lv_obj_set_style_bg_color(line, lv_color_hex(C_ORANGE), 0);
    lv_obj_set_style_bg_grad_color(line, lv_color_hex(C_BG), 0);
    lv_obj_set_style_bg_grad_dir(line, LV_GRAD_DIR_HOR, 0);
    lv_obj_set_style_bg_opa(line, LV_OPA_COVER, 0);
}
// Nut Setting (bo cuc moi): bieu tuong banh rang + co le (lib/RoasterIcons_std.h, sinh boi tools/make_icons.py)
static lv_obj_t *v3_settings_button(lv_obj_t *parent, int x, int y) {
    lv_obj_t *b = v3_button(parent, x, y, UI_BTN, UI_BTN, C_BTN, C_BORDER, "", F_SMALL, C_CREAM);
    static lv_image_dsc_t dsc;
    memset(&dsc, 0, sizeof(dsc));
    dsc.header.magic  = LV_IMAGE_HEADER_MAGIC;
    dsc.header.cf     = LV_COLOR_FORMAT_RGB565A8;
    dsc.header.w      = ICON_SET_PX;
    dsc.header.h      = ICON_SET_PX;
    dsc.header.stride = ICON_SET_PX * 2;
    dsc.data_size     = ICON_SET_PX * ICON_SET_PX * 3;
    dsc.data          = icon_settings_std;
    lv_obj_t *img = lv_image_create(b);
    lv_image_set_src(img, &dsc);
    lv_obj_center(img);
    lv_obj_remove_flag(img, LV_OBJ_FLAG_CLICKABLE);
#if ENABLE_SETTINGS_PAGE
    lv_obj_add_event_cb(b, ev_open_settings, LV_EVENT_CLICKED, NULL);
#endif
    return b;
}

// Hang dieu khien 1 dong (Heater / Air): bieu tuong | - | thanh truot | + | gia tri
//  Heater: cham bieu tuong = bat/tat PID; khi PID: chu "PID" trang de len ngon lua, thanh truot -> "Set <nhiet do>",
//          nut -/+ doi muc nhiet Set (PID_STEP_C do C moi lan).
//  compact = true (khong du rong cho thanh truot, vd trang bieu do NGANG): bieu tuong | - | gia tri | +
//          PID: gia tri doi thanh "Set <nhiet do>" (cam).
static CtlRow v3_ctl_row(int x, int y, int w, bool heater, bool compact = false) {
    CtlRow r = {};
    lv_obj_t *c = v3_card(x, y, w, UI_ROW_H);
    const int iy = (UI_ROW_H - UI_ICON) / 2;
    const int by = (UI_ROW_H - UI_BTN) / 2;
    r.icon = make_icon(c, heater ? ICON_FLAME : ICON_FAN, ISZ_STD, UI_PAD_X, iy);
    const int mx = UI_PAD_X + UI_ICON + UI_GAP;            // nut -
    lv_event_cb_t cb = heater ? ev_heater_step : ev_air_step;
    const int step = heater ? STEP_HEATER : STEP_AIR;
    int sx, sw;                                              // vung giua (thanh truot / chu Set)
    make_step_button(c, LV_SYMBOL_MINUS, mx, by, UI_BTN, cb, -step, true, UI_BTN, UI_RADIUS);
    if (compact) {
        const int px = w - UI_PAD_X - UI_BTN;                // nut + sat mep phai
        make_step_button(c, LV_SYMBOL_PLUS, px, by, UI_BTN, cb, step, true, UI_BTN, UI_RADIUS);
        sx = mx + UI_BTN + UI_GAP; sw = px - UI_GAP - sx;
        r.slider = nullptr;
        r.value = make_label(c, "0%", F_MED, C_CREAM);
        lv_obj_set_width(r.value, sw);
        lv_obj_set_style_text_align(r.value, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_pos(r.value, sx, (UI_ROW_H - (int)lv_font_get_line_height(F_MED)) / 2);
    } else {
        const int vw = 50, vx = w - UI_PAD_X - vw;           // gia tri (canh phai)
        const int px = vx - UI_GAP - UI_BTN;                 // nut +
        make_step_button(c, LV_SYMBOL_PLUS, px, by, UI_BTN, cb, step, true, UI_BTN, UI_RADIUS);
        sx = mx + UI_BTN + 10; sw = px - 10 - sx;
        r.slider = make_slider(c, sx, (UI_ROW_H - 14) / 2, sw, heater ? 0xFFC107 : 0x4DD0E1, heater ? 0xFF3D00 : 0x2979FF,
                               heater ? ev_heater_slider : ev_air_slider);
        r.value = make_label(c, "0%", F_MED, C_CREAM);
        lv_obj_set_width(r.value, vw);
        lv_obj_set_style_text_align(r.value, LV_TEXT_ALIGN_RIGHT, 0);
        lv_obj_set_pos(r.value, vx, (UI_ROW_H - (int)lv_font_get_line_height(F_MED)) / 2);
    }
    if (heater) {
        const lv_font_t *sf = fnt(F_NORM);
        r.sv = make_label(c, "", sf, UI_C_HEATER);               // "Set 200°C" thay thanh truot khi PID
        lv_obj_set_width(r.sv, sw);
        lv_label_set_long_mode(r.sv, LV_LABEL_LONG_CLIP);
        lv_obj_set_style_text_align(r.sv, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_pos(r.sv, sx, (UI_ROW_H - (int)lv_font_get_line_height(sf)) / 2);
        lv_obj_add_flag(r.sv, LV_OBJ_FLAG_HIDDEN);
        r.pidlb = make_label(c, "PID", F_SMALL, 0xFFFFFF);        // chu PID de len ngon lua
        lv_obj_set_width(r.pidlb, UI_ICON);
        lv_obj_set_style_text_align(r.pidlb, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_pos(r.pidlb, UI_PAD_X, iy + (UI_ICON - (int)lv_font_get_line_height(F_SMALL)) / 2);
        lv_obj_add_flag(r.pidlb, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(r.icon, LV_OBJ_FLAG_CLICKABLE);          // cham bieu tuong = bat/tat PID
        lv_obj_set_ext_click_area(r.icon, 6);
        lv_obj_set_style_opa(r.icon, LV_OPA_70, LV_STATE_PRESSED);
        lv_obj_add_event_cb(r.icon, ev_pid_toggle, LV_EVENT_CLICKED, NULL);
    }
    return r;
}

static void v3_ctl_apply(CtlRow &r, const Snap &s, bool heater, int &c_pid) {
    if (!r.value) return;
    char buf[32];
    if (heater) {
        if ((int)s.pid != c_pid) {
            c_pid = s.pid;
            lv_obj_t *mid = r.slider ? r.slider : r.value;     // thanh truot (hang day du) hoac gia tri (hang gon) nhuong cho
            if (s.pid) { lv_obj_add_flag(mid, LV_OBJ_FLAG_HIDDEN);    lv_obj_remove_flag(r.sv, LV_OBJ_FLAG_HIDDEN); lv_obj_remove_flag(r.pidlb, LV_OBJ_FLAG_HIDDEN); }
            else       { lv_obj_remove_flag(mid, LV_OBJ_FLAG_HIDDEN); lv_obj_add_flag(r.sv, LV_OBJ_FLAG_HIDDEN);    lv_obj_add_flag(r.pidlb, LV_OBJ_FLAG_HIDDEN); }
            // PID: ngon lua toi bot de chu PID trang noi ro
            lv_obj_set_style_image_recolor(r.icon, lv_color_hex(UI_C_BG), 0);
            lv_obj_set_style_image_recolor_opa(r.icon, s.pid ? LV_OPA_40 : LV_OPA_TRANSP, 0);
            lv_obj_set_style_text_color(r.value, lv_color_hex(s.pid ? UI_C_HEATER : UI_C_CREAM), 0);
        }
        if (s.pid) {
            snprintf(buf, sizeof(buf), "%s %d%s", tr("Set"), (int)lroundf(to_disp(s.sp)), unit_txt());
            set_text_if_changed(r.sv, buf);
        }
        if (r.slider && !lv_slider_is_dragged(r.slider)) lv_slider_set_value(r.slider, s.heat, LV_ANIM_OFF);
        snprintf(buf, sizeof(buf), "%d%%", s.heat);
    } else {
        if (r.slider && !lv_slider_is_dragged(r.slider)) lv_slider_set_value(r.slider, s.vent, LV_ANIM_OFF);
        snprintf(buf, sizeof(buf), "%d%%", s.vent);
    }
    set_text_if_changed(r.value, buf);
}

// The Drum / Cool: bieu tuong | cong tac | ON/OFF (cham ca the de bat/tat)
static void v3_toggle_card(int x, int y, int w, bool isDrum, lv_event_cb_t sw_cb, lv_obj_t **out_sw, lv_obj_t **out_lbl) {
    lv_obj_t *c = v3_card(x, y, w, UI_ROW_H);
    const int iy = (UI_ROW_H - UI_ICON) / 2;
    lv_obj_t *icn = make_icon(c, isDrum ? ICON_DRUM : ICON_SNOW, ISZ_STD, UI_PAD_X, iy);
    if (isDrum) { ic_drum = icn; c_ic_drum = -1; } else { ic_cool = icn; c_ic_cool = -1; }
    const int swx = UI_PAD_X + UI_ICON + UI_GAP, swy = (UI_ROW_H - UI_SWITCH_H) / 2;
    lv_obj_t *sw = lv_switch_create(c);
    lv_obj_set_pos(sw, swx, swy);
    lv_obj_set_size(sw, UI_SWITCH_W, UI_SWITCH_H);
    lv_obj_set_style_bg_color(sw, lv_color_hex(C_TRACK), LV_PART_MAIN);
    lv_obj_set_style_bg_color(sw, lv_color_hex(isDrum ? UI_C_DRUM : UI_C_COOL), LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(sw, lv_color_hex(C_CREAM), LV_PART_KNOB);
    lv_obj_add_event_cb(sw, sw_cb, LV_EVENT_VALUE_CHANGED, NULL);
    const lv_font_t *lf = fnt(F_NORM);
    lv_obj_t *l = make_label(c, tr("OFF"), lf, C_MUTED);
    lv_obj_set_pos(l, swx + UI_SWITCH_W + 8, (UI_ROW_H - (int)lv_font_get_line_height(lf)) / 2);
    lv_obj_add_event_cb(c, ev_card_tap, LV_EVENT_CLICKED, sw);
    *out_sw = sw;
    *out_lbl = l;
}

// ---- Hop thoai xac nhan (tranh bam nham): ui_confirm("Cau hoi", "Nut dong y", ham) ----
static int g_conf_kind = 0;                 // 0 = hop thoai thuong; 1 = Drop co dem nguoc; 2 = END co dem nguoc
static void confirm_timed_end() { g_conf_kind = 0; }
static void (*g_confirm_cb)() = nullptr;
static lv_obj_t *g_confirm_ov = nullptr;
static void confirm_close() { if (g_confirm_ov) { lv_obj_delete_async(g_confirm_ov); g_confirm_ov = nullptr; } }
static void ev_confirm_ok(lv_event_t *)     { void (*cb)() = g_confirm_cb; confirm_timed_end(); confirm_close(); if (cb) cb(); ui_poll(); }
static void ev_confirm_cancel(lv_event_t *) { if (g_conf_kind) panelCancelConfirm(); confirm_timed_end(); confirm_close(); }
static void ui_confirm(const char *msg, const char *ok_txt, void (*cb)(), lv_obj_t **out_ok_lb = nullptr) {
    if (g_confirm_ov) return;
    g_confirm_cb = cb;
    lv_obj_t *ov = lv_obj_create(lv_layer_top());
    lv_obj_remove_style_all(ov);
    lv_obj_set_size(ov, ui_w(), ui_h());
    lv_obj_set_style_bg_color(ov, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(ov, LV_OPA_60, 0);
    lv_obj_add_flag(ov, LV_OBJ_FLAG_CLICKABLE);               // chan cham xuong trang ben duoi
    lv_obj_remove_flag(ov, LV_OBJ_FLAG_SCROLLABLE);
    const int bw = 264, bh = 132;
    lv_obj_t *box = lv_obj_create(ov);
    lv_obj_remove_style_all(box);
    lv_obj_set_size(box, bw, bh);
    lv_obj_center(box);
    lv_obj_set_style_bg_color(box, lv_color_hex(UI_C_CARD), 0);
    lv_obj_set_style_bg_opa(box, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(box, UI_RADIUS, 0);
    lv_obj_set_style_border_width(box, 1, 0);
    lv_obj_set_style_border_color(box, lv_color_hex(UI_C_BORDER), 0);
    lv_obj_remove_flag(box, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_t *m = make_label(box, msg, fnt(F_NORM), UI_C_CREAM);
    lv_obj_set_width(m, bw - 2 * 14);
    lv_label_set_long_mode(m, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_align(m, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_pos(m, 14, 20);
    const int btw = (bw - 2 * 12 - UI_GAP) / 2, bty = bh - 12 - UI_BTN;
    lv_obj_t *bc = v3_button(box, 12, bty, btw, UI_BTN, UI_C_BTN, UI_C_BORDER, tr("Cancel"), fnt(F_NORM), UI_C_CREAM);
    lv_obj_add_event_cb(bc, ev_confirm_cancel, LV_EVENT_CLICKED, NULL);
    lv_obj_t *bo = v3_button(box, 12 + btw + UI_GAP, bty, btw, UI_BTN, UI_C_DANGER, UI_C_DANGER, ok_txt, fnt(F_NORM), 0xFFFFFF, out_ok_lb);
    lv_obj_add_event_cb(bo, ev_confirm_ok, LV_EVENT_CLICKED, NULL);
    g_confirm_ov = ov;
}

// ---- Hop thoai Drop / END co dem nguoc DROP_CONFIRM_MS / END_CONFIRM_MS (SkiCMD.h giu bo dem; het gio tu dong huy) ----
static lv_obj_t *g_conf_okl = nullptr;
static void conf_do_drop() { Snap s; roasterSnapshot(s); if (s.dropArmMs > 0) panelDrop(); }   // con trong thoi gian cho moi xac nhan
static void conf_do_end()  { Snap s; roasterSnapshot(s); if (s.endArmMs  > 0) panelEnd();  }
static void ui_confirm_timed(int kind) {
    if (g_confirm_ov) return;
    g_conf_kind = kind;
    char buf[24];
    snprintf(buf, sizeof(buf), "%s %us", kind == 1 ? "Drop" : tr("END"), (unsigned)(kind == 1 ? DROP_CONFIRM_MS : END_CONFIRM_MS) / 1000);
    ui_confirm(tr(kind == 1 ? "Drop beans?" : "End roast?"), buf, kind == 1 ? conf_do_drop : conf_do_end, &g_conf_okl);
}
static void confirm_timed_tick(const Snap &s) {
    if (!g_conf_kind) return;
    const uint32_t ms = (g_conf_kind == 1) ? s.dropArmMs : s.endArmMs;
    if (ms == 0 || !g_confirm_ov) { g_conf_kind = 0; confirm_close(); return; }      // het gio (hoac da xong): dong, coi nhu huy
    char buf[24];
    snprintf(buf, sizeof(buf), "%s %us", g_conf_kind == 1 ? "Drop" : tr("END"), (unsigned)((ms + 999) / 1000));
    set_text_if_changed(g_conf_okl, buf);
}

// The ket noi HiBean / Artisan: huy hieu tron (logo goc) + trang thai Off / Wait / Connected.
//  HiBean: cham the -> dang bat thi hoi xac nhan roi tat (xam); dang tat thi bat ngay.  Artisan: chua noi chuc nang (luon Off).
static void hibean_disable() { hibeanWanted = false; }
static void ev_hibean_card(lv_event_t *) {
    if (hibeanWanted) ui_confirm(tr("Disable HiBean?"), tr("Disable"), hibean_disable);
    else { hibeanWanted = true; ui_poll(); }
}
static void v3_link_card(int x, int y, int w, bool hibean) {
    lv_obj_t *c = v3_card(x, y, w, UI_ROW_H);
    const int iy = (UI_ROW_H - UI_ICON) / 2;
    lv_obj_t *icn = nullptr;
    icn = make_icon(c, hibean ? ICON_HIBEAN : ICON_ARTISAN, ISZ_STD, UI_PAD_X, iy);
    const lv_font_t *lf = fnt(F_NORM);
    lv_obj_t *l = make_label(c, tr("Off"), lf, UI_C_MUTED);
    lv_obj_set_pos(l, UI_PAD_X + UI_ICON + UI_GAP, (UI_ROW_H - (int)lv_font_get_line_height(lf)) / 2);
    if (hibean) {
        card_hibean = c; lb_link = l; logo_hibean = icn;
        lv_obj_add_flag(c, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_opa(c, LV_OPA_70, LV_STATE_PRESSED);
        lv_obj_add_event_cb(c, ev_hibean_card, LV_EVENT_CLICKED, NULL);
    } else {
        if (icn) icon_set_dim(icn, false);
        lv_obj_remove_flag(c, LV_OBJ_FLAG_CLICKABLE);
    }
}

#if ENABLE_PROFILE
#include "RoasterProfile.h"     // du lieu Profile rang mau
#endif
#if ENABLE_ROAST_GRAPH
#include "RoasterGraph.h"      // MODULE bieu do rang (tat bang ENABLE_ROAST_GRAPH 0: trang chinh dung lai cum 6 nut moc)
#endif

#if (ENABLE_START_DIALOG && ENABLE_PROFILE)
#include "RoasterStartMode.h"  // hop thoai START: Manual / Auto
#endif


// =============================================================================
// 7a. TRANG CHINH - BO CUC MOI (DOC 320 x 480)
//  Tieu de 48 px | the nhiet do + nut Graph | Heater | Air | Drum + Cool | HiBean + Artisan
//  Moi so lieu kich thuoc lay tu lib/UIConfig.h.
// =============================================================================
static void create_main_ui_v3() {
    const int W = ui_w(), H = ui_h();
    const int CW = W - 2 * UI_MARGIN;                       // be rong noi dung
    lv_obj_t *scr = lv_screen_active();

    // ---- Tieu de: ten may + trang thai | Setting | START/END ----
    lv_obj_t *title = make_label(scr, "LY GIA ROASTER", F_MED, C_CREAM);
    lv_obj_set_pos(title, UI_MARGIN, UI_MARGIN - 2);           // ten + trang thai nam gon trong chieu cao nut tieu de
    lb_status = make_label(scr, "", fnt(F_SMALL), C_MUTED);
    lv_obj_set_pos(lb_status, UI_MARGIN, UI_MARGIN + UI_BTN - (int)lv_font_get_line_height(F_SMALL));
    const int by = UI_MARGIN;
    const int sx = W - UI_MARGIN - UI_START_W;
    btn_start = v3_button(scr, sx, by, UI_START_W, UI_BTN, C_GREEN, C_GREEN, tr("START"), fnt(F_MED), C_BG, &lb_start);
    lv_obj_add_event_cb(btn_start, ev_start, LV_EVENT_CLICKED, NULL);
    int firstx = sx;
#if ENABLE_SETTINGS_PAGE
    firstx = sx - UI_GAP - UI_BTN;
    v3_settings_button(scr, firstx, by);
#endif
    g_hint_right = firstx;
    lb_stage = make_label(scr, "", fnt(F_SMALL), C_RED);    // canh bao (vd "Press START") tren dong trang thai
    lv_obj_set_width(lb_stage, 100);
    lv_label_set_long_mode(lb_stage, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_set_style_text_align(lb_stage, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_pos(lb_stage, 60, lv_obj_get_y(lb_status));
    v3_header_line(W);

    // ---- Chia chieu cao: phan con lai sau 4 hang dieu khien danh cho the nhiet do ----
    const int y0 = UI_HEADER_H + UI_GAP;
    const int rows_h = 4 * UI_ROW_H + 4 * UI_GAP;          // Heater, Air, Drum/Cool, HiBean/Artisan (+ khe phia tren moi hang)
    const int th = (H - UI_MARGIN) - y0 - rows_h;           // chieu cao the nhiet do (= nut Graph)

    // ---- The nhiet do: BT | so nhiet lon | Time | RoR ----
    const int tw = CW - UI_GAP - UI_GRAPH_W;
    lv_obj_t *tc = v3_card(UI_MARGIN, y0, tw, th);
    lb_bt_title = make_label(tc, "BT", fnt(F_SMALL), C_MUTED);
    lv_obj_set_pos(lb_bt_title, UI_PAD_X, 6);
    char tbuf[16];
    snprintf(tbuf, sizeof(tbuf), "--.-%s", unit_txt());
    const int lbig = (int)lv_font_get_line_height(F_BIG);
    const int vy = th - UI_PAD_Y - lbig;                      // dong gia tri Time / RoR (sat day the)
    const int cy = vy - 18;                                  // dong chu thich
    lb_temp = make_label(tc, tbuf, F_HUGE, C_MUTED);          // so nhiet lon: can giua khoang tu nhan BT den vach ngang
    {
        const int top = 6 + (int)lv_font_get_line_height(F_SMALL), bot = cy - 8;
        const int ty = top + (bot - top - (int)lv_font_get_line_height(F_HUGE)) / 2;
        lv_obj_set_pos(lb_temp, UI_PAD_X, ty > top ? ty : top);
    }
    lv_obj_t *dv = lv_obj_create(tc);                        // vach ngang
    lv_obj_remove_style_all(dv);
    lv_obj_set_size(dv, tw - 2 * UI_PAD_X, 1);
    lv_obj_set_pos(dv, UI_PAD_X, cy - 8);
    lv_obj_set_style_bg_color(dv, lv_color_hex(C_BORDER), 0);
    lv_obj_set_style_bg_opa(dv, LV_OPA_COVER, 0);
    const int half = (tw - 2 * UI_PAD_X) / 2;
    lv_obj_t *dv2 = lv_obj_create(tc);                       // vach doc giua Time | RoR
    lv_obj_remove_style_all(dv2);
    lv_obj_set_size(dv2, 1, th - (cy - 2) - UI_PAD_Y);
    lv_obj_set_pos(dv2, UI_PAD_X + half, cy - 2);
    lv_obj_set_style_bg_color(dv2, lv_color_hex(C_BORDER), 0);
    lv_obj_set_style_bg_opa(dv2, LV_OPA_COVER, 0);
    char rcap[24];
    snprintf(rcap, sizeof(rcap), "RoR %s/%s", unit_txt(), tr("min"));
    lv_obj_t *cap_time = make_label(tc, tr("Time"), fnt(F_SMALL), C_MUTED);
    lv_obj_set_pos(cap_time, UI_PAD_X, cy);
    lv_obj_t *cap_ror = make_label(tc, rcap, fnt(F_SMALL), C_MUTED);
    lv_obj_set_pos(cap_ror, UI_PAD_X + half + 8, cy);
    lb_timer = make_label(tc, "00:00", F_BIG, C_MUTED);
    lv_obj_set_pos(lb_timer, UI_PAD_X, vy);
    lb_ror = make_label(tc, "--", F_BIG, C_MUTED);
    lv_obj_set_pos(lb_ror, UI_PAD_X + half + 8, vy);

    // ---- Nut Graph: ben phai the nhiet do, cao bang the ----
#if ENABLE_ROAST_GRAPH
    build_graph_button_v3(UI_MARGIN + tw + UI_GAP, y0, UI_GRAPH_W, th);
#endif

    // ---- Hang dieu khien ----
    int y = y0 + th + UI_GAP;
    m_heat = v3_ctl_row(UI_MARGIN, y, CW, true);   y += UI_ROW_H + UI_GAP;
    m_air  = v3_ctl_row(UI_MARGIN, y, CW, false);  y += UI_ROW_H + UI_GAP;
    const int hw = (CW - UI_GAP) / 2;
    v3_toggle_card(UI_MARGIN, y, hw, true,  ev_drum_sw, &sw_drum, &lb_drum);
    v3_toggle_card(UI_MARGIN + hw + UI_GAP, y, CW - hw - UI_GAP, false, ev_cool_sw, &sw_cool, &lb_cool);
    y += UI_ROW_H + UI_GAP;
    v3_link_card(UI_MARGIN, y, hw, true);
    v3_link_card(UI_MARGIN + hw + UI_GAP, y, CW - hw - UI_GAP, false);
    c_m_pid = -1;
}

// =============================================================================
// 7b. TRANG CHINH - BO CUC MOI (NGANG 480 x 320)
//  Luoi 2 cot thang hang: cot trai 318 px | cot phai 142 px (UI_COL_R)
//    [ The nhiet do: BT | so nhiet lon || Time / RoR ]  (trai ra het chieu ngang, vach chia thang cot phai)
//    [ Heater                    ] [ Drum  ]
//    [ Air                       ] [ Cool  ]
//    [ HiBean      ] [ Artisan   ] [ Graph ]
// =============================================================================
static void create_main_ui_v3L() {
    const int W = ui_w(), H = ui_h();
    const int CW = W - 2 * UI_MARGIN;
    const int RW = UI_COL_R, LW = CW - UI_GAP - RW;           // cot phai / cot trai
    const int RX = UI_MARGIN + LW + UI_GAP;
    lv_obj_t *scr = lv_screen_active();

    // ---- Tieu de (giong trang doc) ----
    lv_obj_t *title = make_label(scr, "LY GIA ROASTER", F_MED, C_CREAM);
    lv_obj_set_pos(title, UI_MARGIN, UI_MARGIN - 2);           // ten + trang thai nam gon trong chieu cao nut tieu de
    lb_status = make_label(scr, "", fnt(F_SMALL), C_MUTED);
    lv_obj_set_pos(lb_status, UI_MARGIN, UI_MARGIN + UI_BTN - (int)lv_font_get_line_height(F_SMALL));
    const int by = UI_MARGIN;
    const int sx = W - UI_MARGIN - UI_START_W;
    btn_start = v3_button(scr, sx, by, UI_START_W, UI_BTN, C_GREEN, C_GREEN, tr("START"), fnt(F_MED), C_BG, &lb_start);
    lv_obj_add_event_cb(btn_start, ev_start, LV_EVENT_CLICKED, NULL);
    int firstx = sx;
#if ENABLE_SETTINGS_PAGE
    firstx = sx - UI_GAP - UI_BTN;
    v3_settings_button(scr, firstx, by);
#endif
    g_hint_right = firstx;
    lb_stage = make_label(scr, "", fnt(F_SMALL), C_RED);
    lv_obj_set_width(lb_stage, 100);
    lv_label_set_long_mode(lb_stage, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_set_style_text_align(lb_stage, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_pos(lb_stage, 60, lv_obj_get_y(lb_status));
    v3_header_line(W);

    const int y0 = UI_HEADER_H + UI_GAP;
    const int th = (H - UI_MARGIN) - y0 - 3 * (UI_ROW_H + UI_GAP);   // the nhiet do: phan con lai sau 3 hang

    // ---- The nhiet do (ngang het noi dung): vung trai = BT + so nhiet, vung phai (thang cot phai) = Time / RoR ----
    lv_obj_t *tc = v3_card(UI_MARGIN, y0, CW, th);
    lb_bt_title = make_label(tc, "BT", fnt(F_SMALL), C_MUTED);
    lv_obj_set_pos(lb_bt_title, UI_PAD_X, 3);
    char tbuf[16];
    snprintf(tbuf, sizeof(tbuf), "--.-%s", unit_txt());
    lb_temp = make_label(tc, tbuf, F_HUGE, C_MUTED);
    {   // so nhiet: sat day the (phan tren chua nhan BT); the thap thi chen nhe len vung nhan
        const int lh = (int)lv_font_get_line_height(F_HUGE);
        int ty = th - lh - 2;
        const int top = 3 + (int)lv_font_get_line_height(F_SMALL) - 6;
        if (ty < top) ty = top;
        lv_obj_set_pos(lb_temp, UI_PAD_X, ty);
    }
    const int zx = LW + UI_GAP / 2;                           // vach chia (giua khe cot trai / cot phai)
    lv_obj_t *dv = lv_obj_create(tc);
    lv_obj_remove_style_all(dv);
    lv_obj_set_size(dv, 1, th - 2 * UI_PAD_Y);
    lv_obj_set_pos(dv, zx, UI_PAD_Y);
    lv_obj_set_style_bg_color(dv, lv_color_hex(C_BORDER), 0);
    lv_obj_set_style_bg_opa(dv, LV_OPA_COVER, 0);
    // Time / RoR: 2 dong, moi dong = nhan (trai) + gia tri 28 px (phai)
    const int lbig = (int)lv_font_get_line_height(F_BIG), ls = (int)lv_font_get_line_height(F_SMALL);
    const int rx0 = RX - UI_MARGIN + UI_PAD_X - 2, rw = CW - (RX - UI_MARGIN) - UI_PAD_X;   // vung Time/RoR trong the
    const int rowh = (th - 2) / 2;
    char rcap[24];
    snprintf(rcap, sizeof(rcap), "RoR %s/%s", unit_txt(), tr("m"));
    const char *caps[2] = { tr("Time"), rcap };
    for (int i = 0; i < 2; i++) {
        const int ry = 1 + i * rowh;
        lv_obj_t *cp = make_label(tc, caps[i], fnt(F_SMALL), C_MUTED);
        lv_obj_set_pos(cp, rx0, ry + (rowh - ls) / 2);
        lv_obj_t *v = make_label(tc, i ? "--" : "00:00", F_BIG, C_MUTED);
        lv_obj_set_width(v, rw - 4);
        lv_obj_set_style_text_align(v, LV_TEXT_ALIGN_RIGHT, 0);
        lv_obj_set_pos(v, rx0, ry + (rowh - lbig) / 2);
        if (i) lb_ror = v; else lb_timer = v;
    }
    lv_obj_t *dh = lv_obj_create(tc);                        // vach ngang giua Time / RoR
    lv_obj_remove_style_all(dh);
    lv_obj_set_size(dh, rw - 4, 1);
    lv_obj_set_pos(dh, rx0, 1 + rowh);
    lv_obj_set_style_bg_color(dh, lv_color_hex(C_BORDER), 0);
    lv_obj_set_style_bg_opa(dh, LV_OPA_COVER, 0);

    // ---- Hang dieu khien + the nho (thang cot) ----
    int y = y0 + th + UI_GAP;
    m_heat = v3_ctl_row(UI_MARGIN, y, LW, true);
    v3_toggle_card(RX, y, RW, true, ev_drum_sw, &sw_drum, &lb_drum);
    y += UI_ROW_H + UI_GAP;
    m_air = v3_ctl_row(UI_MARGIN, y, LW, false);
    v3_toggle_card(RX, y, RW, false, ev_cool_sw, &sw_cool, &lb_cool);
    y += UI_ROW_H + UI_GAP;
    const int hw = (LW - UI_GAP) / 2;
    v3_link_card(UI_MARGIN, y, hw, true);
    v3_link_card(UI_MARGIN + hw + UI_GAP, y, LW - hw - UI_GAP, false);
#if ENABLE_ROAST_GRAPH
    build_graph_button_v3(RX, y, RW, UI_ROW_H);
#endif
    c_m_pid = -1;
}

// =============================================================================
// 7. TRANG CHINH (doc 320x480 hoac ngang 480x320)
// =============================================================================
static void create_main_ui() {
    const bool L = ui_landscape();
    // con tro cua bo cuc khac: xoa de ui_apply khong dung nham widget da huy
    card_hibean = nullptr;
    m_heat = CtlRow{}; m_air = CtlRow{};
    if (!L) create_main_ui_v3(); else create_main_ui_v3L();
}

// =============================================================================
// 8. CHUYEN TRANG / DUNG LAI GIAO DIEN / DOI HUONG
// =============================================================================
static void ui_apply_orientation() {
    g_applied_orient = g_set.orient;
    gfx->fillScreen(0x0000);
    lv_display_set_resolution(g_disp, ui_w(), ui_h());
    lv_obj_invalidate(lv_screen_active());
}

// Chay trong lv_async_call de khong xoa widget ngay trong su kien cua chinh no
static void ui_rebuild_cb(void *) {
    g_main_built = false;
#if ENABLE_ROAST_GRAPH
    g_graph_built = false;
#endif
    if (g_applied_orient != g_set.orient) ui_apply_orientation();
    lv_obj_clean(lv_screen_active());
    c_last_key = -99; c_last_mode = -1; c_last_tstate = -1;
    c_last_ror = -1; c_start_key = -1;
    ic_drum = ic_cool = nullptr; c_ic_drum = c_ic_cool = -1;
    for (int i = 0; i < EV_COUNT; i++) { c_ev_state[i] = -1; c_ev_en[i] = -1; c_ev_tm[i] = -1; }

#if ENABLE_SETTINGS_PAGE
    if (g_page == PAGE_SETTINGS) { ui_build_settings(); return; }
#endif
#if ENABLE_ROAST_GRAPH
    if (g_page == PAGE_GRAPH) { ui_build_graph(); return; }
#endif
    D_println("UI: create_main_ui");
    create_main_ui();
    g_main_built = true;
    ui_poll();
}

static void ui_request_page(int page) {
    g_page = page;
    lv_async_call(ui_rebuild_cb, NULL);
}

// =============================================================================
// 9. DRIVER LVGL 9: MAN HINH + CAM UNG
// =============================================================================
static uint32_t my_tick() { return millis(); }

static void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
    const int32_t w = area->x2 - area->x1 + 1;
    const int32_t h = area->y2 - area->y1 + 1;

    if (!ui_landscape()) {
        gfx->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t *)px_map, w, h);
    } else {
        // Xoay 90 do bang phan mem: (lx,ly) ngang -> (px,py) doc, ghi thang vao framebuffer canvas
        uint16_t *fb = gfx->getFramebuffer();
        const uint16_t *src = (const uint16_t *)px_map;
        for (int32_t xi = 0; xi < w; xi++) {
            const int32_t lx = area->x1 + xi;
            for (int32_t yi = 0; yi < h; yi++) {
                const int32_t ly = area->y1 + yi;
#if LANDSCAPE_FLIP
                fb[(SCREEN_H - 1 - lx) * SCREEN_W + ly] = src[yi * w + xi];
#else
                fb[lx * SCREEN_W + (SCREEN_W - 1 - ly)] = src[yi * w + xi];
#endif
            }
        }
    }
    if (lv_display_flush_is_last(disp)) gfx->flush();
    lv_display_flush_ready(disp);
}

static bool touch_read_raw(uint16_t &x, uint16_t &y) {
    static const uint8_t cmd[8] = {0xB5, 0xAB, 0xA5, 0x5A, 0x00, 0x00, 0x00, 0x08};
    uint8_t d[8];

    Wire.beginTransmission(TOUCH_ADDR);
    Wire.write(cmd, sizeof(cmd));
    if (Wire.endTransmission() != 0) return false;
    if (Wire.requestFrom(TOUCH_ADDR, 8) != 8) return false;
    for (int i = 0; i < 8; i++) d[i] = Wire.read();

    if (d[1] == 0 || d[1] > 4) return false;
    x = ((d[2] & 0x0F) << 8) | d[3];
    y = ((d[4] & 0x0F) << 8) | d[5];
    return true;
}

static void my_touch_read(lv_indev_t *, lv_indev_data_t *data) {
    static int32_t last_x = 0, last_y = 0;
    uint16_t x, y;

    if (touch_read_raw(x, y)) {
#if TOUCH_DEBUG && SERIAL_DEBUG
        Serial.printf("touch raw x=%u y=%u\n", x, y);
#endif
#if TOUCH_SWAP_XY
        uint16_t t = x; x = y; y = t;
#endif
        if (x >= SCREEN_W) x = SCREEN_W - 1;
        if (y >= SCREEN_H) y = SCREEN_H - 1;
#if TOUCH_INVERT_X
        x = SCREEN_W - 1 - x;
#endif
#if TOUCH_INVERT_Y
        y = SCREEN_H - 1 - y;
#endif
        if (!ui_landscape()) {
            last_x = x; last_y = y;
        } else {   // anh xa nguoc voi phep xoay trong my_disp_flush
#if LANDSCAPE_FLIP
            last_x = SCREEN_H - 1 - y;
            last_y = x;
#else
            last_x = y;
            last_y = SCREEN_W - 1 - x;
#endif
        }
        data->state = LV_INDEV_STATE_PRESSED;
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }
    data->point.x = last_x;
    data->point.y = last_y;
}

// =============================================================================
// 10. TASK GIAO DIEN (khoi tao + chay tren core 0 de ngat SPI/DMA khong lam
//     sai lech xung bit-bang cua giao thuc may rang tren core 1)
// =============================================================================
static void sync_cb(lv_timer_t *);

static void ui_task(void *) {
    pinMode(TFT_BLK, OUTPUT);
    digitalWrite(TFT_BLK, LOW);

    D_println("UI: task start");
    settingsLoad();                                   // huong / don vi / ngon ngu da luu
    i18n_init_fonts(F_SMALL, F_NORM, F_MED);
    g_applied_orient = g_set.orient;

    D_println("UI: gfx begin");
    if (!gfx->begin()) {
        D_println("Khoi tao man hinh that bai! Kiem tra PSRAM (OPI).");
        for (;;) vTaskDelay(pdMS_TO_TICKS(1000));
    }
    gfx->fillScreen(0x0000);
    gfx->flush();

    Wire.begin(TOUCH_SDA, TOUCH_SCL);
    Wire.setClock(400000);

    D_println("UI: lv_init");
    lv_init();
    lv_tick_set_cb(my_tick);

    // Bo dem ve (2 cai, RAM trong): thu 40 dong; thieu RAM thi giam dan (32 / 24 / 16 / 8 dong) de man hinh van len
    // (BLE, FFat, cac task cung xin RAM trong luc khoi dong; con trong rat it). Nhieu dong hon = ve nhanh hon mot chut.
    uint8_t *buf1 = nullptr, *buf2 = nullptr;
    uint32_t buf_bytes = 0;
    for (int lines = 40; lines >= 8; lines -= 8) {
        buf_bytes = SCREEN_H * lines * sizeof(uint16_t);              // du cho dong rong nhat (480 px khi ngang)
        buf1 = (uint8_t *)heap_caps_malloc(buf_bytes, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
        buf2 = buf1 ? (uint8_t *)heap_caps_malloc(buf_bytes, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT) : nullptr;
        if (buf1 && buf2) break;
        if (buf1) heap_caps_free(buf1);
        buf1 = buf2 = nullptr;
    }
    D_print("LVGL buffer bytes: "); D_println(buf_bytes);
    D_print("Free heap: "); D_println((uint32_t)esp_get_free_heap_size());
    if (!buf1 || !buf2) {
        D_println("Khong du RAM cho buffer LVGL!");
        for (;;) vTaskDelay(pdMS_TO_TICKS(1000));
    }

    g_disp = lv_display_create(ui_w(), ui_h());
    lv_display_set_flush_cb(g_disp, my_disp_flush);
    lv_display_set_buffers(g_disp, buf1, buf2, buf_bytes, LV_DISPLAY_RENDER_MODE_PARTIAL);

    lv_indev_t *indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, my_touch_read);

    lv_obj_t *scr = lv_screen_active();
    lv_obj_set_style_bg_color(scr, lv_color_hex(C_BG), 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);
    lv_obj_remove_flag(scr, LV_OBJ_FLAG_SCROLLABLE);

    create_main_ui();
    g_main_built = true;
    ui_poll();
    lv_timer_create(sync_cb, 100, NULL);

    lv_timer_handler();
    digitalWrite(TFT_BLK, HIGH);
    D_println("UI: ready");

    alarmSoundStart();                                // loa canh bao qua nhiet (task rieng, core 0)

    for (;;) {
        lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}

static void startUI() {
    xTaskCreatePinnedToCore(ui_task, "ui", 20480, NULL, 3, NULL, 0);
}
