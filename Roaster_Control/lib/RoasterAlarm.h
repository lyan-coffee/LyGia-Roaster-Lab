/*
 * GPL v3 - xem LyGia_Roaster.ino
 *
 * MAN HINH CANH BAO QUA NHIET (LVGL) + dieu khien am thanh.
 * Ve tren lv_layer_top() nen hien de len MOI trang (chinh / Setting) va khong bi xoa khi dung lai giao dien.
 *
 * Trang thai:
 *   IDLE   : nhiet do binh thuong
 *   ACTIVE : nhiet do >= muc canh bao -> hien man hinh + keu loa
 *   MUTED  : nguoi dung bam "Dismiss" -> tat tieng + an man hinh, cho den khi nhiet do ha duoi muc - 3 do
 * Nut tren man hinh canh bao:
 *   Heater Off : cat Heater ngay (tat luon PID), tat tieng, giu man hinh den khi nhiet do ha xuong
 *   Dismiss    : tat tieng va an man hinh
 * Man hinh tu dong dong khi nhiet do ha duoi (muc canh bao - ALARM_HYST_C).
 *
 * Phu thuoc (RoasterUI.h khai bao truoc khi include): make_label, fnt, ui_w/ui_h, to_disp, unit_txt, mau C_*, font F_*
 */
#pragma once

#if ENABLE_OVERHEAT_ALARM

enum { AL_IDLE = 0, AL_ACTIVE = 1, AL_MUTED = 2 };
static int       al_state   = AL_IDLE;
static lv_obj_t *al_ov      = nullptr;   // man hinh canh bao
static lv_obj_t *al_lb_temp = nullptr;   // nhiet do hien tren man hinh canh bao

static const uint32_t AL_BG    = 0x1A0707;
static const uint32_t AL_RED   = 0xFF3B30;
static const uint32_t AL_RED_D = 0x4A1210;

// v: 0..1000 (vong lap). Do trong suot dao dong theo hinh tam giac 40..255 (khong dung API "reverse" de tuong thich moi ban LVGL 9)
static void al_pulse_cb(void *obj, int32_t v) {
    int32_t tri = (v < 500) ? v : (1000 - v);
    lv_obj_set_style_border_opa((lv_obj_t *)obj, (lv_opa_t)(40 + tri * 215 / 500), 0);
}

static void ev_al_heater_off(lv_event_t *) {
    panelHeaterOff();
    g_alarm_sound = false;
    g_alarm_test_until = 0;          // dang bam nut "Test" trong Setting thi ket thuc luon che do thu
}
static void ev_al_dismiss(lv_event_t *) {
    g_alarm_sound = false;
    g_alarm_test_until = 0;          // KHONG de che do thu bat lai man hinh canh bao (nguyen nhan treo man hinh truoc day)
    al_state = AL_MUTED;             // alarm_tick se go man hinh (khong xoa doi tuong ngay trong su kien cua no)
}

static lv_obj_t *al_ring(lv_obj_t *parent, int cx, int cy, int d, uint32_t delay_ms) {
    lv_obj_t *r = lv_obj_create(parent);
    lv_obj_remove_style_all(r);
    lv_obj_set_size(r, d, d);
    lv_obj_set_pos(r, cx - d / 2, cy - d / 2);
    lv_obj_set_style_radius(r, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_width(r, 4, 0);
    lv_obj_set_style_border_color(r, lv_color_hex(AL_RED), 0);
    lv_obj_set_style_border_opa(r, LV_OPA_COVER, 0);
    lv_obj_remove_flag(r, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_remove_flag(r, LV_OBJ_FLAG_SCROLLABLE);

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, r);
    lv_anim_set_exec_cb(&a, al_pulse_cb);
    lv_anim_set_values(&a, 0, 1000);
    lv_anim_set_duration(&a, 1600);
    lv_anim_set_delay(&a, delay_ms);
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
    lv_anim_start(&a);
    return r;
}

static lv_obj_t *al_text(lv_obj_t *parent, const char *txt, const lv_font_t *font, uint32_t color,
                         int x, int y, int w, lv_text_align_t align) {
    lv_obj_t *l = make_label(parent, txt, font, color);
    lv_obj_set_width(l, w);
    lv_obj_set_style_text_align(l, align, 0);
    lv_obj_set_pos(l, x, y);
    return l;
}

static lv_obj_t *al_button(lv_obj_t *parent, const char *txt, int x, int y, int w, int h,
                           bool filled, lv_event_cb_t cb) {
    lv_obj_t *b = lv_button_create(parent);
    lv_obj_remove_style_all(b);
    lv_obj_set_pos(b, x, y);
    lv_obj_set_size(b, w, h);
    lv_obj_set_style_radius(b, 14, 0);
    lv_obj_set_style_bg_opa(b, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(b, lv_color_hex(filled ? AL_RED : AL_RED_D), 0);
    lv_obj_set_style_border_width(b, filled ? 0 : 2, 0);
    lv_obj_set_style_border_color(b, lv_color_hex(AL_RED), 0);
    lv_obj_set_style_bg_opa(b, LV_OPA_70, LV_STATE_PRESSED);
    lv_obj_t *l = make_label(b, txt, fnt(F_NORM), filled ? 0xFFFFFF : C_CREAM);
    lv_obj_center(l);
    lv_obj_add_event_cb(b, cb, LV_EVENT_CLICKED, NULL);
    return b;
}

static void al_format_temp(char *buf, size_t n, float tc) {
    snprintf(buf, n, "%.1f%s", to_disp(tc), unit_txt());
}

static void al_destroy() {
    if (al_ov) { lv_obj_delete(al_ov); al_ov = nullptr; al_lb_temp = nullptr; }
}

static void al_build(float shown_c) {
    al_destroy();                    // luon xoa ban cu truoc khi dung ban moi (tranh de lai man hinh "ma" che het cam ung)
    const bool L = ui_landscape();
    const int W = ui_w(), H = ui_h();
    char buf[48];

    al_ov = lv_obj_create(lv_layer_top());
    lv_obj_remove_style_all(al_ov);
    lv_obj_set_pos(al_ov, 0, 0);
    lv_obj_set_size(al_ov, W, H);
    lv_obj_set_style_bg_color(al_ov, lv_color_hex(AL_BG), 0);
    lv_obj_set_style_bg_opa(al_ov, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(al_ov, 5, 0);
    lv_obj_set_style_border_color(al_ov, lv_color_hex(AL_RED), 0);
    lv_obj_set_style_border_opa(al_ov, LV_OPA_COVER, 0);
    lv_obj_add_flag(al_ov, LV_OBJ_FLAG_CLICKABLE);      // chan cham xuyen xuong trang ben duoi
    lv_obj_remove_flag(al_ov, LV_OBJ_FLAG_SCROLLABLE);

    // ---- Bieu tuong: dau cham than trong hinh tron, hai vong xung nhip ----
    const int icx = L ? 100 : W / 2;
    const int icy = L ? H / 2 : 108;
    al_ring(al_ov, icx, icy, 168, 0);
    al_ring(al_ov, icx, icy, 132, 400);
    lv_obj_t *disc = lv_obj_create(al_ov);
    lv_obj_remove_style_all(disc);
    lv_obj_set_size(disc, 96, 96);
    lv_obj_set_pos(disc, icx - 48, icy - 48);
    lv_obj_set_style_radius(disc, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(disc, lv_color_hex(AL_RED), 0);
    lv_obj_set_style_bg_opa(disc, LV_OPA_COVER, 0);
    lv_obj_remove_flag(disc, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_t *ex = make_label(disc, "!", F_HUGE, 0xFFFFFF);
    lv_obj_center(ex);

    // ---- Chu: tieu de, nhiet do, nguong, loi nhac ----
    const int tx = L ? 196 : 16;
    const int tw = L ? W - tx - 14 : W - 32;
    const lv_text_align_t ta = L ? LV_TEXT_ALIGN_LEFT : LV_TEXT_ALIGN_CENTER;
    int y = L ? 24 : 206;

    al_text(al_ov, tr("OVERHEAT"), fnt(F_MED), AL_RED, tx, y, tw, ta);
    y += L ? 40 : 38;

    al_format_temp(buf, sizeof(buf), shown_c);
    al_lb_temp = al_text(al_ov, buf, F_HUGE, 0xFFF1E6, tx, y, tw, ta);
    y += L ? 70 : 66;

    snprintf(buf, sizeof(buf), "%s %.0f%s", tr("Limit"), to_disp((float)g_set.warn), unit_txt());
    al_text(al_ov, buf, fnt(F_NORM), C_MUTED, tx, y, tw, ta);
    y += 30;

    lv_obj_t *adv = al_text(al_ov, tr("Reduce heater power now"), fnt(F_NORM), C_CREAM, tx, y, tw, ta);
    lv_label_set_long_mode(adv, LV_LABEL_LONG_WRAP);

    // ---- Nut: Heater Off / Dismiss ----
    if (!L) {
        const int bw = (W - 12 * 2 - 14) / 2, bh = 56, by = H - bh - 16;
        al_button(al_ov, tr("Heater Off"), 12,            by, bw, bh, true,  ev_al_heater_off);
        al_button(al_ov, tr("Dismiss"),    12 + bw + 14, by, bw, bh, false, ev_al_dismiss);
    } else {
        const int bw = (tw - 12) / 2, bh = 52, by = H - bh - 16;
        al_button(al_ov, tr("Heater Off"), tx,           by, bw, bh, true,  ev_al_heater_off);
        al_button(al_ov, tr("Dismiss"),    tx + bw + 12, by, bw, bh, false, ev_al_dismiss);
    }
}

// Goi moi 100 ms tu sync_cb (moi trang), bang anh chup trang thai may
static void alarm_tick(const Snap &s) {
    const float warn = (float)g_set.warn;
    const uint32_t tu = g_alarm_test_until;
    const bool test = (tu != 0) && ((int32_t)(tu - millis()) > 0);
    if (tu != 0 && !test) g_alarm_test_until = 0;

    const bool hot  = s.tempOk && s.temp >= warn;
    const bool warm = s.tempOk && s.temp >= warn - ALARM_HYST_C;

    switch (al_state) {
    case AL_IDLE:
        if (hot || test) {
            al_state = AL_ACTIVE;
            al_build(s.tempOk ? s.temp : warn);
            g_alarm_sound = true;
        }
        break;
    case AL_ACTIVE:
        if (!warm && !test) {
            al_state = AL_IDLE;
            g_alarm_sound = false;
        } else if (al_lb_temp) {
            char buf[24];
            al_format_temp(buf, sizeof(buf), s.tempOk ? s.temp : warn);
            if (strcmp(lv_label_get_text(al_lb_temp), buf) != 0) lv_label_set_text(al_lb_temp, buf);
        }
        break;
    case AL_MUTED:
        if (test) {
            al_state = AL_ACTIVE;
            al_build(s.tempOk ? s.temp : warn);
            g_alarm_sound = true;
        } else if (!warm) {
            al_state = AL_IDLE;
        }
        break;
    }

    if (al_state != AL_ACTIVE && al_ov) al_destroy();
}

#else
static void alarm_tick(const Snap &) {}
#endif
