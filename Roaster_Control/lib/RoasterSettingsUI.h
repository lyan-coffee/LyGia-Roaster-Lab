/*
 * GPL v3 - xem LyGia_Roaster.ino
 *
 * TRANG SETTING: huong man hinh, don vi nhiet do, ngon ngu, bao qua nhiet, mo phong, man hinh cho.
 * Chi duoc include khi ENABLE_SETTINGS_PAGE = 1 (xem RoasterUI.h).
 * Phu thuoc (khai bao trong RoasterUI.h truoc khi include file nay):
 *   make_label, make_card, fnt, ui_request_page, PAGE_MAIN/PAGE_SETTINGS, mau C_*, font F_*
 */
#pragma once

// Nut banh rang tren man hinh chinh
static void ev_open_settings(lv_event_t *) { ui_request_page(PAGE_SETTINGS); }
static void ev_close_settings(lv_event_t *) { ui_request_page(PAGE_MAIN); }
#if ENABLE_ABOUT_PAGE
static void ev_open_about(lv_event_t *) { idle_show(true); }       // RoasterIdle.h
#endif

// Chon huong / don vi / ngon ngu: luu, roi dung lai trang Setting
static void ev_set_orient(lv_event_t *e) {
    int v = (int)(intptr_t)lv_event_get_user_data(e);
    if (g_set.orient == v) return;
    g_set.orient = v;
    settingsSave();
    ui_request_page(PAGE_SETTINGS);      // ui_rebuild se ap dung huong moi
}
static void ev_set_unit(lv_event_t *e) {
    int v = (int)(intptr_t)lv_event_get_user_data(e);
    if (g_set.unit == v) return;
    g_set.unit = v;
    settingsSave();
    ui_request_page(PAGE_SETTINGS);
}
static void ev_set_lang(lv_event_t *e) {
    int v = (int)(intptr_t)lv_event_get_user_data(e);
    if (g_set.lang == v || !i18n_lang_usable(v)) return;
    g_set.lang = v;
    settingsSave();
    ui_request_page(PAGE_SETTINGS);
}

#if ENABLE_SIMULATION
static void ev_set_sim(lv_event_t *e) {
    const int idx = (int)(intptr_t)lv_event_get_user_data(e);
    if (idx == 2) {                            // nut xN: doi he so thoi gian gia lap theo vong 1 -> 3 -> 5 -> 7
        static const uint8_t SC[] = { 1, 3, 5, 7 };
        int k = 0;
        while (k < 3 && SC[k] != g_simScale) k++;
        g_simScale = SC[(k + 1) % 4];
        settingsSave();
        ui_request_page(PAGE_SETTINGS);
        return;
    }
    bool on = idx == 1;
    if (on && realSignalOk()) return;          // co tin hieu that: khong cho gia lap
    g_simWanted = on;
    ui_request_page(PAGE_SETTINGS);            // dung lai de cap nhat nut chon
}
#endif

#if ENABLE_IDLE_SCREEN
static void ev_set_idle(lv_event_t *e) {
    int v = (int)(intptr_t)lv_event_get_user_data(e);
    if (g_set.idle == v) return;
    g_set.idle = (uint8_t)v;
    settingsSave();
    ui_request_page(PAGE_SETTINGS);
}
#endif

#if ENABLE_OVERHEAT_ALARM
static lv_obj_t *lb_warn_val = nullptr;
static void warn_refresh() {
    if (!lb_warn_val) return;
    char b[16];
    snprintf(b, sizeof(b), "%d%s", (int)lroundf(to_disp((float)g_set.warn)), unit_txt());
    lv_label_set_text(lb_warn_val, b);
}
static void ev_warn_step(lv_event_t *e) {
    int d = (int)(intptr_t)lv_event_get_user_data(e);
    int v = constrain((int)g_set.warn + d, WARN_TEMP_MIN, WARN_TEMP_MAX);
    if (v == (int)g_set.warn) return;
    g_set.warn = (uint16_t)v;
    settingsSave();
    warn_refresh();
}
// Nut "Test": hien man hinh canh bao + keu loa 4 giay de kiem tra
static void ev_warn_test(lv_event_t *) { g_alarm_test_until = millis() + 4000; }

#endif


// =============================================================================
// TRANG SETTING - BO CUC MOI (QuyTac_GiaoDien.txt, so lieu lib/UIConfig.h)
//  Tieu de: [<] SETTINGS (nut 40 x 40 tu le 10 px), duong ke y = UI_HEADER_H, noi dung tu y = 62.
//  The: bo 6 px, nhan 14 px mau MUTED canh trai (dem 7 / 6 px), hang nut 40 px sat day the (dem 6 px).
//  Chieu cao the tu chia deu phan con lai -> le duoi dung 10 px.
//  DOC : Huong | Don vi | Ngon ngu | Bao qua nhiet | [Mo phong | Man hinh cho]   (300 px; hang cuoi 2 cot 147 | 147)
//  NGANG: [Huong | Don vi] / [Ngon ngu | Man hinh cho] / [Bao qua nhiet | Mo phong]  (luoi 312 | 142)
//  Thieu mot the (tat bang co trong LySetting.h / UIConfig.h): the con lai trong hang chiem ca hang.
// =============================================================================
static lv_obj_t *set3_btn(lv_obj_t *p, const char *txt, const lv_font_t *f, int x, int y, int w,
                          bool sel, bool en, lv_event_cb_t cb, int v) {
    lv_obj_t *b = lv_button_create(p);
    lv_obj_remove_style_all(b);
    lv_obj_set_pos(b, x, y);
    lv_obj_set_size(b, w, UI_BTN);
    lv_obj_set_style_bg_color(b, lv_color_hex(sel ? C_ORANGE : C_BTN), 0);
    lv_obj_set_style_bg_opa(b, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(b, UI_RADIUS, 0);
    lv_obj_set_style_border_width(b, 1, 0);
    lv_obj_set_style_border_color(b, lv_color_hex(sel ? C_ORANGE : C_BORDER), 0);
    if (!sel && en) lv_obj_set_style_bg_color(b, lv_color_hex(C_ORANGE), LV_STATE_PRESSED);
    if (!en) lv_obj_set_style_opa(b, LV_OPA_40, 0);
    lv_obj_center(make_label(b, txt, f, sel ? C_BG : C_CREAM));
    if (en && cb) lv_obj_add_event_cb(b, cb, LV_EVENT_CLICKED, (void *)(intptr_t)v);
    return b;
}

// The: nhan + tra ve y cua hang nut (sat day)
static lv_obj_t *set3_card(int x, int y, int w, int h, const char *title, int &by) {
    lv_obj_t *c = make_card(x, y, w, h);
    lv_obj_set_style_radius(c, UI_RADIUS, 0);
    lv_obj_set_pos(make_label(c, title, fnt(F_SMALL), C_MUTED), UI_PAD_X, UI_PAD_Y);
    by = h - UI_PAD_Y - UI_BTN;
    return c;
}

// The co n nut chon chia deu be rong
static void set3_choice(int x, int y, int w, int h, const char *title, int n, const char *const *labels,
                        const lv_font_t *const *fonts, const bool *en, int selected, lv_event_cb_t cb) {
    int by;
    lv_obj_t *c = set3_card(x, y, w, h, title, by);
    const int inner = w - 2 * UI_PAD_X;
    const int bw = (inner - UI_GAP * (n - 1)) / n;
    const int extra = inner - (bw * n + UI_GAP * (n - 1));      // phan du: cong vao nut cuoi -> thang mep phai
    for (int i = 0; i < n; i++) {
        set3_btn(c, labels[i], fonts[i], UI_PAD_X + i * (bw + UI_GAP), by, bw + (i == n - 1 ? extra : 0),
                 i == selected, en ? en[i] : true, cb, i);
    }
}

#if ENABLE_OVERHEAT_ALARM
// The bao qua nhiet: [-] 240°C [+] [Thu]  (moi nut 40 px)
static void set3_warn(int x, int y, int w, int h) {
    int by;
    lv_obj_t *c = set3_card(x, y, w, h, tr("Overheat Alarm"), by);
    const int tw = 64;                                          // nut Thu
    const int tx = w - UI_PAD_X - tw;
    const int px = tx - UI_GAP - UI_BTN;                        // nut +
    const int mx = UI_PAD_X;                                    // nut -
    make_step_button(c, LV_SYMBOL_MINUS, mx, by, UI_BTN, ev_warn_step, -WARN_TEMP_STEP, false, UI_BTN, UI_RADIUS);
    make_step_button(c, LV_SYMBOL_PLUS,  px, by, UI_BTN, ev_warn_step,  WARN_TEMP_STEP, false, UI_BTN, UI_RADIUS);
    set3_btn(c, tr("Test"), fnt(F_SMALL), tx, by, tw, false, true, ev_warn_test, 0);
    const lv_font_t *vf = fnt(F_MED);
    const int vx = mx + UI_BTN + UI_GAP, vw = px - UI_GAP - vx;
    lb_warn_val = make_label(c, "", vf, C_ORANGE);
    lv_obj_set_width(lb_warn_val, vw);
    lv_obj_set_style_text_align(lb_warn_val, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_long_mode(lb_warn_val, LV_LABEL_LONG_CLIP);
    lv_obj_set_pos(lb_warn_val, vx, by + (UI_BTN - (int)lv_font_get_line_height(vf)) / 2);
    warn_refresh();
}
#endif

#if ENABLE_PID_SETTINGS
// =============================================================================
// PID: 6 nut = Soft | Balanced | Fast (co dinh) va P1 | P2 | P3 (cua nguoi dung: bam vao mo hop thoai sua
// Kp / Ki / Kd / kieu P, chon Save de luu + ap dung, hoac Don't Save de bo).
// Chon nut co dinh = ap dung ngay + luu bo dang dung. Nut sang cam = bo trung voi thong so dang chay.
// =============================================================================
static lv_obj_t *pid_btn[6];
static lv_obj_t *g_pd_ov = nullptr;                      // hop thoai sua P1..P3
static float    pd_k[3];                                 // ban nhap (Kp, Ki, Kd)
static uint8_t  pd_pm;
static int      pd_slot;
static lv_obj_t *pd_val[3], *pd_pm_btn[2];

static void pid_btn_style(lv_obj_t *b, bool sel) {
    lv_obj_set_style_bg_color(b, lv_color_hex(sel ? C_ORANGE : C_BTN), 0);
    lv_obj_set_style_border_color(b, lv_color_hex(sel ? C_ORANGE : C_BORDER), 0);
    lv_obj_set_style_bg_color(b, lv_color_hex(C_ORANGE), LV_STATE_PRESSED);
    lv_obj_t *l = lv_obj_get_child(b, 0);
    if (l) lv_obj_set_style_text_color(l, lv_color_hex(sel ? C_BG : C_CREAM), 0);
}
static bool pid_same(const PidPreset &r) {
    return fabsf(r.kp - (float)myPIDConfig.getKp()) < 0.01f && fabsf(r.ki - (float)myPIDConfig.getKi()) < 0.001f &&
           fabsf(r.kd - (float)myPIDConfig.getKd()) < 0.01f && r.pm == (myPIDConfig.getPMode() == P_ON_M ? 1 : 0);
}
static void pid_refresh() {
    bool found = false;
    for (int i = 0; i < 6; i++) if (pid_btn[i]) {
        const bool m = !found && pid_same(i < 3 ? PID_PRESETS[i] : g_pidUser[i - 3]);
        if (m) found = true;
        pid_btn_style(pid_btn[i], m);
    }
}
static void ev_pid_fixed(lv_event_t *e) {
    const PidPreset &r = PID_PRESETS[(int)(intptr_t)lv_event_get_user_data(e)];
    pidCfgApply(r.kp, r.ki, r.kd, r.pm);
    pidSettingsSave();
    pid_refresh();
}

// ---- hop thoai sua P1..P3 ----
static void pd_close() { if (g_pd_ov) { lv_obj_delete_async(g_pd_ov); g_pd_ov = nullptr; } }
// Nut chon kieu P: dang "segmented control" (mot khung bo tron, 2 phan; phan dang chon to mau cam) - khac nut hanh dong
static void pd_seg_style(lv_obj_t *sg, bool sel) {
    lv_obj_set_style_bg_opa(sg, sel ? LV_OPA_COVER : LV_OPA_TRANSP, 0);
    lv_obj_set_style_bg_color(sg, lv_color_hex(C_ORANGE), 0);
    lv_obj_t *l = lv_obj_get_child(sg, 0);
    if (l) lv_obj_set_style_text_color(l, lv_color_hex(sel ? C_BG : C_MUTED), 0);
}
static void pd_refresh() {
    char b[16];
    snprintf(b, sizeof b, "%.0f", pd_k[0]); lv_label_set_text(pd_val[0], b);
    snprintf(b, sizeof b, "%.2f", pd_k[1]); lv_label_set_text(pd_val[1], b);
    snprintf(b, sizeof b, "%.1f", pd_k[2]); lv_label_set_text(pd_val[2], b);
    for (int i = 0; i < 2; i++) pd_seg_style(pd_pm_btn[i], i == pd_pm);
}
static void ev_pd_step(lv_event_t *e) {                 // user data: hang * 10 + (huong + 1)
    const int ud = (int)(intptr_t)lv_event_get_user_data(e), row = ud / 10, dir = ud % 10 - 1;
    if (row == 0) pd_k[0] = constrain(pd_k[0] + dir * 1.0f, 0.0f, 50.0f);
    if (row == 1) pd_k[1] = constrain(roundf((pd_k[1] + dir * 0.05f) * 100.0f) / 100.0f, 0.0f, 2.0f);
    if (row == 2) pd_k[2] = constrain(roundf((pd_k[2] + dir * (pd_k[2] + (dir < 0 ? 0.0f : 0.01f) < 10.0f ? 0.5f : 5.0f)) * 10.0f) / 10.0f, 0.0f, 100.0f);   // Kd: buoc 0.5 duoi 10, buoc 5 tu 10
    pd_refresh();
}
static void ev_pd_pm(lv_event_t *e) { pd_pm = (uint8_t)(intptr_t)lv_event_get_user_data(e); pd_refresh(); }
static void ev_pd_cancel(lv_event_t *) { pd_close(); }
static void ev_pd_save(lv_event_t *) {
    PidPreset &u = g_pidUser[pd_slot];
    u.kp = pd_k[0]; u.ki = pd_k[1]; u.kd = pd_k[2]; u.pm = pd_pm;
    pidUserSave(pd_slot);                                   // luu bo cua nguoi dung
    pidCfgApply(u.kp, u.ki, u.kd, u.pm);                    // va dung ngay
    pidSettingsSave();
    pd_close();
    pid_refresh();
}
static void ev_pid_user(lv_event_t *e) {
    if (g_pd_ov) return;
    pd_slot = (int)(intptr_t)lv_event_get_user_data(e);
    const PidPreset &u = g_pidUser[pd_slot];
    pd_k[0] = u.kp; pd_k[1] = u.ki; pd_k[2] = u.kd; pd_pm = u.pm;
    const int bw = 280, RB = 36, RG = 4, TOP = 44;
    const int bh = TOP + 3 * (RB + RG) + RB + 12 + UI_BTN + 12;
    lv_obj_t *ov = lv_obj_create(lv_layer_top());
    lv_obj_remove_style_all(ov);
    lv_obj_set_size(ov, ui_w(), ui_h());
    lv_obj_set_style_bg_color(ov, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(ov, LV_OPA_60, 0);
    lv_obj_add_flag(ov, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_remove_flag(ov, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_t *box = lv_obj_create(ov);
    lv_obj_remove_style_all(box);
    lv_obj_set_size(box, bw, bh);
    lv_obj_center(box);
    lv_obj_set_style_bg_color(box, lv_color_hex(C_CARD), 0);
    lv_obj_set_style_bg_opa(box, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(box, UI_RADIUS, 0);
    lv_obj_set_style_border_width(box, 1, 0);
    lv_obj_set_style_border_color(box, lv_color_hex(C_BORDER), 0);
    lv_obj_remove_flag(box, LV_OBJ_FLAG_SCROLLABLE);
    char tt[24]; snprintf(tt, sizeof tt, "PID  %s", u.name);
    lv_obj_t *t = make_label(box, tt, fnt(F_NORM), C_ORANGE);
    lv_obj_set_width(t, bw - 28); lv_obj_set_style_text_align(t, LV_TEXT_ALIGN_CENTER, 0); lv_obj_set_pos(t, 14, 12);
    lv_obj_t *ln = lv_obj_create(box);
    lv_obj_remove_style_all(ln);
    lv_obj_set_pos(ln, 12, 12 + 20 + 6); lv_obj_set_size(ln, bw - 24, 2);
    lv_obj_set_style_bg_color(ln, lv_color_hex(C_ORANGE), 0);
    lv_obj_set_style_bg_grad_color(ln, lv_color_hex(C_CARD), 0);
    lv_obj_set_style_bg_grad_dir(ln, LV_GRAD_DIR_HOR, 0);
    lv_obj_set_style_bg_opa(ln, LV_OPA_COVER, 0);
    static const char *const NM[3] = { "Kp", "Ki", "Kd" };
    const lv_font_t *vf = fnt(F_MED), *nf = fnt(F_NORM);
    for (int r = 0; r < 3; r++) {
        const int ry = TOP + r * (RB + RG);
        lv_obj_set_pos(make_label(box, NM[r], nf, C_MUTED), 16, ry + (RB - (int)lv_font_get_line_height(nf)) / 2);
        const int mx = 58, px = bw - 14 - RB;
        make_step_button(box, LV_SYMBOL_MINUS, mx, ry, RB, ev_pd_step, r * 10 + 0, true, RB, UI_RADIUS);
        make_step_button(box, LV_SYMBOL_PLUS,  px, ry, RB, ev_pd_step, r * 10 + 2, true, RB, UI_RADIUS);
        pd_val[r] = make_label(box, "", vf, C_ORANGE);
        const int vx = mx + RB + UI_GAP, vw = px - UI_GAP - vx;
        lv_obj_set_width(pd_val[r], vw); lv_obj_set_style_text_align(pd_val[r], LV_TEXT_ALIGN_CENTER, 0);
        lv_label_set_long_mode(pd_val[r], LV_LABEL_LONG_CLIP);
        lv_obj_set_pos(pd_val[r], vx, ry + (RB - (int)lv_font_get_line_height(vf)) / 2);
    }
    const int my = TOP + 3 * (RB + RG), mbw = (bw - 28 - UI_GAP) / 2;
    {   // hang "P": nhan ben trai thang cot voi Kp / Ki / Kd, khung chon 2 phan chiem phan con lai
        lv_obj_set_pos(make_label(box, "P", nf, C_MUTED), 16, my + (RB - (int)lv_font_get_line_height(nf)) / 2);
        const int sx = 58, sw = bw - 14 - sx, pad = 3, gw = (sw - 2 * pad) / 2;
        lv_obj_t *seg = lv_obj_create(box);
        lv_obj_remove_style_all(seg);
        lv_obj_set_pos(seg, sx, my);
        lv_obj_set_size(seg, sw, RB);
        lv_obj_set_style_bg_color(seg, lv_color_hex(C_BTN), 0);
        lv_obj_set_style_bg_opa(seg, LV_OPA_COVER, 0);
        lv_obj_set_style_radius(seg, UI_RADIUS, 0);
        lv_obj_set_style_border_width(seg, 1, 0);
        lv_obj_set_style_border_color(seg, lv_color_hex(C_BORDER), 0);
        lv_obj_remove_flag(seg, LV_OBJ_FLAG_SCROLLABLE);
        const char *tx[2] = { tr("P on Error"), tr("P on Meas") };
        for (int i = 0; i < 2; i++) {
            lv_obj_t *sg = lv_obj_create(seg);
            lv_obj_remove_style_all(sg);
            lv_obj_set_pos(sg, pad - 1 + i * gw + (i ? (sw - 2 * pad) - 2 * gw : 0), pad - 1);
            lv_obj_set_size(sg, gw + (i ? 0 : (sw - 2 * pad) - 2 * gw), RB - 2 * pad);
            lv_obj_set_style_radius(sg, UI_RADIUS - 3, 0);
            lv_obj_set_style_bg_opa(sg, LV_OPA_TRANSP, 0);
            lv_obj_remove_flag(sg, LV_OBJ_FLAG_SCROLLABLE);
            lv_obj_add_flag(sg, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_center(make_label(sg, tx[i], fnt(F_SMALL), C_MUTED));
            lv_obj_add_event_cb(sg, ev_pd_pm, LV_EVENT_CLICKED, (void *)(intptr_t)i);
            pd_pm_btn[i] = sg;
        }
    }
    // Chan hop thoai: [Don't Save] vien manh, chu mo  |  [Save] nut cam dac (hanh dong chinh)
    const int fy = my + RB + 12;
    lv_obj_t *c = set3_btn(box, tr("Don't Save"), fnt(F_SMALL), 14, fy, mbw, false, true, ev_pd_cancel, 0);
    lv_obj_set_style_bg_opa(c, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_color(c, lv_color_hex(C_MUTED), 0);
    lv_obj_set_style_text_color(lv_obj_get_child(c, 0), lv_color_hex(C_CREAM), 0);
    set3_btn(box, tr("Save"), fnt(F_SMALL), 14 + mbw + UI_GAP, fy, mbw, true, true, ev_pd_save, 0);
    pd_refresh();
    g_pd_ov = ov;
}

// The PID: Soft | Balanced | Fast / P1 | P2 | P3 (doc: 2 hang x 3 nut; ngang: 1 hang x 6 nut)
static void set3_pid(int x, int y, int w, int h, bool oneRow) {
    int by;
    lv_obj_t *c = set3_card(x, y, w, h, "PID", by);
    const int inner = w - 2 * UI_PAD_X;
    const int cols = oneRow ? 6 : 3;
    const int bw = (inner - UI_GAP * (cols - 1)) / cols, extra = inner - (bw * cols + UI_GAP * (cols - 1));
    for (int i = 0; i < 6; i++) {
        const int col = oneRow ? i : i % 3;
        const int ry = oneRow ? by : (i < 3 ? by - UI_BTN - UI_GAP : by);
        const int bx = UI_PAD_X + col * (bw + UI_GAP), wd = bw + (col == cols - 1 ? extra : 0);
        pid_btn[i] = set3_btn(c, i < 3 ? tr(PID_PRESETS[i].name) : g_pidUser[i - 3].name, fnt(F_SMALL), bx, ry, wd, false, true,
                              i < 3 ? ev_pid_fixed : ev_pid_user, i < 3 ? i : i - 3);
    }
    pid_refresh();
}
#endif

static void ui_build_settings() {
    const bool L = ui_landscape();
    const int W = ui_w(), H = ui_h();
    const int CW = W - 2 * UI_MARGIN;
    lv_obj_t *scr = lv_screen_active();

    // ---- Tieu de: [<]  SETTINGS ----
    lv_obj_t *back = set3_btn(scr, LV_SYMBOL_LEFT, F_MED, UI_MARGIN, UI_MARGIN, UI_BTN, false, true, ev_close_settings, 0);
    (void)back;
#if ENABLE_ABOUT_PAGE
    {   // Nut "i" (Gioi thieu): cung hang chu SETTINGS, sat le phai, 40 x 40 nhu nut quay lai
        lv_obj_t *ab = set3_btn(scr, "", F_MED, W - UI_MARGIN - UI_BTN, UI_MARGIN, UI_BTN, false, true, ev_open_about, 0);
        lv_obj_t *ring = lv_obj_create(ab);
        lv_obj_remove_style_all(ring);
        lv_obj_set_size(ring, 22, 22);
        lv_obj_center(ring);
        lv_obj_set_style_radius(ring, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_border_width(ring, 2, 0);
        lv_obj_set_style_border_color(ring, lv_color_hex(C_CREAM), 0);
        lv_obj_remove_flag(ring, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_center(make_label(ring, "i", fnt(F_SMALL), C_CREAM));
    }
#endif
    const lv_font_t *tf = fnt(F_MED);
    lv_obj_t *title = make_label(scr, tr("SETTINGS"), tf, C_CREAM);
    lv_obj_set_pos(title, UI_MARGIN + UI_BTN + UI_MARGIN, UI_MARGIN + (UI_BTN - (int)lv_font_get_line_height(tf)) / 2);
    {
        lv_obj_t *line = lv_obj_create(scr);
        lv_obj_remove_style_all(line);
        lv_obj_set_pos(line, UI_MARGIN, UI_HEADER_H);
        lv_obj_set_size(line, CW, 2);
        lv_obj_set_style_bg_color(line, lv_color_hex(C_ORANGE), 0);
        lv_obj_set_style_bg_grad_color(line, lv_color_hex(C_BG), 0);
        lv_obj_set_style_bg_grad_dir(line, LV_GRAD_DIR_HOR, 0);
        lv_obj_set_style_bg_opa(line, LV_OPA_COVER, 0);
    }
#if ENABLE_OVERHEAT_ALARM
    lb_warn_val = nullptr;
#endif

    // ---- Danh sach the ----
    enum { S_ORI, S_UNIT, S_LANG, S_WARN, S_SIM, S_IDLE, S_N };
    bool has[S_N] = { true, true, true, false, false, false };
#if ENABLE_OVERHEAT_ALARM
    has[S_WARN] = true;
#endif
#if ENABLE_SIMULATION
    has[S_SIM] = true;
#endif
#if ENABLE_IDLE_SCREEN
    has[S_IDLE] = true;
#endif
    // Hang: [the trai, the phai]. Thieu the nao thi bo; con 1 the thi chiem ca hang.
    static const int8_t ROWS_P[4][2] = { {S_ORI, S_UNIT}, {S_LANG, -1}, {S_WARN, -1}, {S_SIM, S_IDLE} };
    static const int8_t ROWS_L[3][2] = { {S_ORI, S_UNIT}, {S_LANG, S_IDLE}, {S_WARN, S_SIM} };
    const int8_t (*src)[2] = L ? ROWS_L : ROWS_P;
    const int nsrc = L ? 3 : 4;
    int rows[S_N][2]; int nr = 0;
    for (int i = 0; i < nsrc; i++) {
        int a = src[i][0], b = src[i][1];
        if (a >= 0 && !has[a]) a = -1;
        if (b >= 0 && !has[b]) b = -1;
        if (a < 0) { a = b; b = -1; }
        if (a < 0) continue;
        rows[nr][0] = a; rows[nr][1] = b; nr++;
    }
    const int y0 = UI_HEADER_H + UI_GAP;
#if ENABLE_PID_SETTINGS
    const int npid = 1, pidExtra = L ? 0 : UI_BTN + UI_GAP;     // the PID: doc cao hon 1 hang nut (2 hang x 3 nut), ngang 1 hang x 6 nut
#else
    const int npid = 0, pidExtra = 0;
#endif
    const int avail = (H - UI_MARGIN) - y0 - UI_GAP * (nr + npid - 1) - pidExtra;
    int chb = avail / (nr + npid), rem = avail % (nr + npid);   // phan du chia cho cac hang dau (moi hang +1 px)
    if (chb < 68) { chb = 68; rem = 0; }                        // the thap nhat 68 px (nhan + nut 40 px); thieu cho thi keo cuon

    auto build = [&](int id, int x, int y, int w, int h) {
        switch (id) {
        case S_ORI: {
            const char *lb[2] = { tr("Portrait"), tr("Landscape") };
            const lv_font_t *ft[2] = { fnt(F_SMALL), fnt(F_SMALL) };
            set3_choice(x, y, w, h, tr("Orientation"), 2, lb, ft, nullptr, g_set.orient, ev_set_orient);
        } break;
        case S_UNIT: {
            const char *lb[2] = { "°C", "°F" };
            const lv_font_t *ft[2] = { F_NORM, F_NORM };
            set3_choice(x, y, w, h, tr("Temp Unit"), 2, lb, ft, nullptr, g_set.unit, ev_set_unit);
        } break;
        case S_LANG: {                                           // moi nut hien bang chinh ngon ngu cua no
            const char *lb[3] = { "English", g_vi_ok ? "Tiếng Việt" : "Tieng Viet", g_zh_ok ? "中文" : "Chinese" };
            const lv_font_t *ft[3] = { F_SMALL, i18n_font_lang(LANG_VI, 0, F_SMALL), i18n_font_lang(LANG_ZH, 0, F_SMALL) };
            bool en[3] = { true, true, i18n_lang_usable(LANG_ZH) };
            set3_choice(x, y, w, h, tr("Language"), 3, lb, ft, en, g_set.lang, ev_set_lang);
        } break;
#if ENABLE_OVERHEAT_ALARM
        case S_WARN: set3_warn(x, y, w, h); break;
#endif
#if ENABLE_SIMULATION
        case S_SIM: {                                            // chi bat duoc khi khong co tin hieu that
            char sx[8]; snprintf(sx, sizeof sx, "x%d", (int)g_simScale);
            const char *lb[3] = { tr("OFF"), tr("ON"), sx };
            const lv_font_t *ft[3] = { fnt(F_NORM), fnt(F_NORM), fnt(F_NORM) };
            bool en[3] = { true, g_simWanted || !realSignalOk(), true };
            set3_choice(x, y, w, h, tr("Simulation"), 3, lb, ft, en, g_simWanted ? 1 : 0, ev_set_sim);
        } break;
#endif
#if ENABLE_IDLE_SCREEN
        case S_IDLE: {
            const char *lb[2] = { tr("OFF"), tr("ON") };
            const lv_font_t *ft[2] = { fnt(F_NORM), fnt(F_NORM) };
            set3_choice(x, y, w, h, tr("Idle Screen"), 2, lb, ft, nullptr, g_set.idle ? 1 : 0, ev_set_idle);
        } break;
#endif
        default: break;
        }
    };

    // Vung cuon (keo len / xuong): chua tat ca cac the; y trong vung = y man hinh - y0
    lv_obj_t *sc = lv_obj_create(scr);
    lv_obj_remove_style_all(sc);
    lv_obj_set_pos(sc, 0, y0);
    lv_obj_set_size(sc, W, H - y0);
    lv_obj_add_flag(sc, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scroll_dir(sc, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(sc, LV_SCROLLBAR_MODE_AUTO);
    lv_obj_set_style_pad_bottom(sc, UI_MARGIN, 0);
    lv_obj_set_style_bg_color(sc, lv_color_hex(C_ORANGE), LV_PART_SCROLLBAR);
    lv_obj_set_style_bg_opa(sc, LV_OPA_70, LV_PART_SCROLLBAR);
    lv_obj_set_style_width(sc, 3, LV_PART_SCROLLBAR);
    lv_obj_set_style_radius(sc, 2, LV_PART_SCROLLBAR);
    g_card_parent = sc;
    int y = 0;
    for (int r = 0; r < nr; r++) {
        const int h = chb + (r < rem ? 1 : 0);
        if (rows[r][1] < 0) {
            build(rows[r][0], UI_MARGIN, y, CW, h);
        } else {                                                 // 2 cot: NGANG 312 | 142 (UI_COL_R); DOC chia doi
            const int rw = L ? UI_COL_R : (rows[r][0] == S_ORI ? 104 : (CW - UI_GAP) / 2);   // doc: Huong rong | Don vi hep
            const int lw = CW - UI_GAP - rw;
            build(rows[r][0], UI_MARGIN, y, lw, h);
            build(rows[r][1], UI_MARGIN + lw + UI_GAP, y, rw, h);
        }
        y += h + UI_GAP;
    }
#if ENABLE_PID_SETTINGS
    set3_pid(UI_MARGIN, y, CW, chb + pidExtra, L);
#endif
    g_card_parent = nullptr;
    lv_obj_update_layout(sc);
    if (lv_obj_get_scroll_bottom(sc) <= 4) lv_obj_remove_flag(sc, LV_OBJ_FLAG_SCROLLABLE);   // vua du man hinh: khong cuon, khong hien thanh cuon
}

