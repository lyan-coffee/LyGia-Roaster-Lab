/*
 * GPL v3 - xem LyGia_Roaster.ino
 *
 * HOP THOAI BAM START: chon cach rang (Manual / Auto). Bat/tat: ENABLE_START_DIALOG (LySetting.h).
 *   Manual : START roi sang trang Graph; nguoi dung tu bam nut PR (PID giu PRESET_PR_TEMP_C = 180, Air 10).
 *   Auto   : mo danh sach Profile rang mau -> chon 1 profile. (Dong co chay profile: chua co, se lam sau - hien chi START.)
 *   Co me dang mo (nut dang la END) thi KHONG hien hop thoai nay.
 * Phu thuoc (RoasterUI.h khai bao truoc): v3_button, make_label, fnt, ui_w/ui_h, ui_request_page, PAGE_GRAPH, panelStart, panelEvent, tr, UI_*
 */
#pragma once

#if (ENABLE_START_DIALOG && ENABLE_PROFILE)

// Danh sach Profile: lay tu PROFILE_FACTORY (RoasterProfile.h). Hop thoai nay chi co khi ENABLE_PROFILE = 1
#define SM_PROFILE_COUNT PROFILE_COUNT
#define SM_PNAME(i)   (PROFILE_FACTORY[i].name)
#define SM_PWEIGHT(i) (PROFILE_FACTORY[i].weight)
#define SM_PPROC(i)   (PROFILE_FACTORY[i].process)
#define SM_PLEVEL(i)  (PROFILE_FACTORY[i].level)
#define SM_VISIBLE_ROWS  4                       // so dong thay duoc cung luc, nhieu hon thi keo len / xuong

static lv_obj_t *g_sm_ov = nullptr;
static void sm_close() { if (g_sm_ov) { lv_obj_delete_async(g_sm_ov); g_sm_ov = nullptr; } }

// Nen toi mo phu man hinh + hop 264 x bh o giua (cung kieu ui_confirm)
static lv_obj_t *sm_box(int bh, lv_obj_t **ov_out) {
    lv_obj_t *ov = lv_obj_create(lv_layer_top());
    lv_obj_remove_style_all(ov);
    lv_obj_set_size(ov, ui_w(), ui_h());
    lv_obj_set_style_bg_color(ov, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(ov, LV_OPA_60, 0);
    lv_obj_add_flag(ov, LV_OBJ_FLAG_CLICKABLE);                        // chan cham xuong trang ben duoi
    lv_obj_remove_flag(ov, LV_OBJ_FLAG_SCROLLABLE);
    const int bw = 264;
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
    *ov_out = ov;
    return box;
}
// Tieu de hop thoai: bieu tuong + chu mau cam, duong ke chuyen mau ngay duoi (cung kieu tieu de trang)
static void sm_title(lv_obj_t *box, const char *icon, const char *txt) {
    char buf[64];
    snprintf(buf, sizeof(buf), "%s  %s", icon, txt);
    lv_obj_t *m = make_label(box, buf, fnt(F_NORM), C_ORANGE);
    lv_obj_set_width(m, 264 - 28);
    lv_obj_set_style_text_align(m, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_pos(m, 14, 14);
    lv_obj_t *ln = lv_obj_create(box);
    lv_obj_remove_style_all(ln);
    lv_obj_set_pos(ln, 12, 14 + 20 + 6);
    lv_obj_set_size(ln, 264 - 24, 2);
    lv_obj_set_style_bg_color(ln, lv_color_hex(C_ORANGE), 0);
    lv_obj_set_style_bg_grad_color(ln, lv_color_hex(UI_C_CARD), 0);
    lv_obj_set_style_bg_grad_dir(ln, LV_GRAD_DIR_HOR, 0);
    lv_obj_set_style_bg_opa(ln, LV_OPA_COVER, 0);
}
#define SM_TOP (14 + 20 + 6 + 2 + 12)                     // y bat dau noi dung (duoi duong ke tieu de)

static void sm_cancel(lv_event_t *) { sm_close(); }
static void sm_manual(lv_event_t *) {
    sm_close();
#if ENABLE_PROFILE
    g_prof_sel = -1;                                                   // Manual: khong co duong mau
#endif
    panelStart();                                                      // START: Drum bat, Air >= muc toi thieu, dong ho me rang
    ui_request_page(PAGE_GRAPH);                                       // Manual: KHONG tu PreHeat; nguoi dung tu bam nut PR (PID 180 C, Air 10)
    ui_poll();
}
#if ENABLE_AUTO_NOTICE
// Thong bao nhe (khong chan thao tac) tren trang Graph: dang PreHeat theo profile; cham vao hoac sau 5 giay se tat
static lv_obj_t *g_sm_note = nullptr;
static void sm_note_close() { if (g_sm_note) { lv_obj_delete_async(g_sm_note); g_sm_note = nullptr; } }
static void sm_note_click(lv_event_t *) { sm_note_close(); }
static void sm_note_timer(lv_timer_t *t) { sm_note_close(); lv_timer_delete(t); }
static void sm_note_show(int i) {
    sm_note_close();
    const int bw = 264, bh = 84;
    lv_obj_t *b = lv_obj_create(lv_layer_top());
    lv_obj_remove_style_all(b);
    lv_obj_set_size(b, bw, bh);
    lv_obj_align(b, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(b, lv_color_hex(UI_C_CARD), 0);
    lv_obj_set_style_bg_opa(b, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(b, UI_RADIUS, 0);
    lv_obj_set_style_border_width(b, 2, 0);
    lv_obj_set_style_border_color(b, lv_color_hex(0xFF8A1F), 0);          // vien cam: dang PreHeat
    lv_obj_remove_flag(b, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(b, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(b, sm_note_click, LV_EVENT_CLICKED, nullptr);
    char buf[48];
    lv_obj_t *l1 = make_label(b, SM_PNAME(i), fnt(F_NORM), UI_C_CREAM);                          // ten profile (tren cung)
    lv_obj_set_width(l1, bw - 16); lv_obj_set_style_text_align(l1, LV_TEXT_ALIGN_CENTER, 0); lv_label_set_long_mode(l1, LV_LABEL_LONG_DOT); lv_obj_set_pos(l1, 6, 8);
    lv_obj_t *l2 = make_label(b, tr("Start PreHeat"), fnt(F_NORM), C_ORANGE);
    lv_obj_set_width(l2, bw - 16); lv_obj_set_style_text_align(l2, LV_TEXT_ALIGN_CENTER, 0); lv_obj_set_pos(l2, 6, 32);
#if ENABLE_PROFILE
    snprintf(buf, sizeof(buf), "%s %.0f%s", tr("Charge at"), to_disp(PROFILE_FACTORY[i].charge10 / 10.0f), unit_txt());
    lv_obj_t *l3 = make_label(b, buf, fnt(F_SMALL), UI_C_MUTED);
    lv_obj_set_width(l3, bw - 16); lv_obj_set_style_text_align(l3, LV_TEXT_ALIGN_CENTER, 0); lv_obj_set_pos(l3, 6, 56);
#endif
    g_sm_note = b;
    lv_timer_t *t = lv_timer_create(sm_note_timer, 5000, nullptr);
    lv_timer_set_repeat_count(t, 1);
}
#endif

static void sm_profile(lv_event_t *e) {
    const int i = (int)(intptr_t)lv_event_get_user_data(e);
#if ENABLE_PROFILE
    g_prof_sel = i;                                                    // Graph ve duong mau; profileRunTick() chay cac luat cua profile nay
    profileRunReset();
#else
    (void)i;
#endif
    sm_close();
    panelStart();
#if ENABLE_PROFILE
    panelEvent(EV_PREHEAT);                                            // danh dau PreHeat; luat Preheat cua profile dat PID + Air (thay cho preset PR)
#endif
    ui_request_page(PAGE_GRAPH);
    ui_poll();
#if ENABLE_AUTO_NOTICE
    sm_note_show(i);                                                   // thong bao dang PreHeat
#endif
}

// Hop thoai 2: danh sach Profile
static void sm_show_profiles() {
    sm_close();
    const int rowH = UI_BTN + 12, gap = UI_GAP;              // dong 2 tang: ten + khoi luong khuyen nghi
    int rows = SM_PROFILE_COUNT < SM_VISIBLE_ROWS ? SM_PROFILE_COUNT : SM_VISIBLE_ROWS;
    while (rows > 1 && SM_TOP + rows * rowH + (rows - 1) * gap + 12 + UI_BTN + 12 > ui_h() - 16) rows--;   // man ngang thap: it dong hon
    const int listH = rows * rowH + (rows - 1) * gap;
    const int bh = SM_TOP + listH + 12 + UI_BTN + 12;
    lv_obj_t *ov;
    lv_obj_t *box = sm_box(bh, &ov);
    sm_title(box, LV_SYMBOL_LIST, tr("Choose profile"));
    lv_obj_t *list = lv_obj_create(box);                              // khung keo (khong thanh cuon)
    lv_obj_remove_style_all(list);
    lv_obj_set_pos(list, 12, SM_TOP);
    lv_obj_set_size(list, 264 - 24, listH);
    lv_obj_add_flag(list, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scroll_dir(list, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(list, LV_SCROLLBAR_MODE_OFF);
    for (int i = 0; i < SM_PROFILE_COUNT; i++) {
        lv_obj_t *nm;
        lv_obj_t *b = v3_button(list, 0, i * (rowH + gap), 264 - 24, rowH, UI_C_BTN, UI_C_BORDER, SM_PNAME(i), fnt(F_NORM), UI_C_CREAM, &nm);
        lv_obj_set_style_border_color(b, lv_color_hex(UI_C_BORDER), 0);
        lv_obj_set_style_border_width(b, 1, 0);
        lv_obj_set_width(nm, 264 - 24 - 48);                                   // ten va khoi luong can trai, mui ten cam ben phai
        lv_obj_set_style_text_align(nm, LV_TEXT_ALIGN_LEFT, 0);
        lv_label_set_long_mode(nm, LV_LABEL_LONG_DOT);
        char info[64]; info[0] = 0;                                            // dong 2: khoi luong | muc rang | so che (bo qua muc trong)
        const char *parts[3] = { SM_PWEIGHT(i), SM_PLEVEL(i), SM_PPROC(i) };
        for (int k = 0; k < 3; k++) {
            if (!parts[k] || !parts[k][0]) continue;
            if (info[0]) strncat(info, " | ", sizeof(info) - strlen(info) - 1);
            strncat(info, parts[k], sizeof(info) - strlen(info) - 1);
        }
        const bool hasw = info[0] != 0;
        lv_obj_align(nm, LV_ALIGN_LEFT_MID, 12, hasw ? -9 : 0);
        if (hasw) {
            lv_obj_t *w = make_label(b, info, fnt(F_SMALL), UI_C_MUTED);
            lv_obj_set_size(w, 264 - 24 - 24, lv_font_get_line_height(fnt(F_SMALL)));   // dong 2 dung het be ngang (mui ten nam o dong 1), dai qua thi "..."
            lv_label_set_long_mode(w, LV_LABEL_LONG_DOT);
            lv_obj_align(w, LV_ALIGN_LEFT_MID, 12, 11);
        }
        lv_obj_t *ar = make_label(b, LV_SYMBOL_RIGHT, fnt(F_NORM), C_ORANGE);
        lv_obj_align(ar, LV_ALIGN_RIGHT_MID, -12, hasw ? -9 : 0);
        lv_obj_add_event_cb(b, sm_profile, LV_EVENT_CLICKED, (void *)(intptr_t)i);
    }
    lv_obj_t *c = v3_button(box, 12, bh - 12 - UI_BTN, 264 - 24, UI_BTN, UI_C_CARD, UI_C_BORDER, tr("Cancel"), fnt(F_NORM), UI_C_MUTED);
    lv_obj_add_event_cb(c, sm_cancel, LV_EVENT_CLICKED, NULL);
    g_sm_ov = ov;
}
static void sm_auto(lv_event_t *) { sm_show_profiles(); }

// Hop thoai 1: Manual | Auto
static void ui_start_choose() {
    if (g_sm_ov) return;
    const int by = UI_BTN + 8;                                         // nut chon cach rang cao hon nut Huy
    const int bh = SM_TOP + by + UI_GAP + UI_BTN + 12;
    lv_obj_t *ov;
    lv_obj_t *box = sm_box(bh, &ov);
    sm_title(box, LV_SYMBOL_PLAY, tr("Start roast"));
    const int bw = (264 - 2 * 12 - UI_GAP) / 2, y = SM_TOP;
    static char mbuf[40], abuf[40];
    snprintf(mbuf, sizeof(mbuf), "%s  %s", LV_SYMBOL_EDIT, tr("Manual"));
    snprintf(abuf, sizeof(abuf), "%s  %s", LV_SYMBOL_LIST, tr("Auto"));
    lv_obj_t *m = v3_button(box, 12, y, bw, by, UI_C_BTN, UI_C_BORDER, mbuf, fnt(F_NORM), UI_C_CREAM);
    lv_obj_add_event_cb(m, sm_manual, LV_EVENT_CLICKED, NULL);
    lv_obj_t *a = v3_button(box, 12 + bw + UI_GAP, y, bw, by, UI_C_BTN, C_ORANGE, abuf, fnt(F_NORM), C_ORANGE);   // Auto: vien + chu cam (chay theo Profile)
    lv_obj_set_style_border_width(a, 2, 0);
    lv_obj_add_event_cb(a, sm_auto, LV_EVENT_CLICKED, NULL);
    lv_obj_t *c = v3_button(box, 12, y + by + UI_GAP, 264 - 24, UI_BTN, UI_C_CARD, UI_C_BORDER, tr("Cancel"), fnt(F_NORM), UI_C_MUTED);
    lv_obj_add_event_cb(c, sm_cancel, LV_EVENT_CLICKED, NULL);
    g_sm_ov = ov;
}

#endif
