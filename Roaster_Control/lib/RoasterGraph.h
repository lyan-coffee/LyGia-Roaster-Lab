/*
 * GPL v3 - xem LyGia_Roaster.ino
 *
 * MODULE BIEU DO RANG - PHAN GIAO DIEN (LVGL). Duoc RoasterUI.h include khi ENABLE_ROAST_GRAPH = 1.
 *  - Trang bieu do: duong BT (cam, truc trai) + duong RoR (xanh, truc phai) manh, moi moc (Charge, DE, 1FC, 2FC, Drop) la mot cham nho tren duong BT,
 *    cum 6 nut moc, chinh Heater / Air. Du lieu tu RoasterGraphData.h.
 *  - Trang chinh (RoasterUI.h) thay cum 6 nut moc bang nut "Roast Graph" (build_graph_button) khi module nay duoc bat.
 *  - Mau sac / font dung lai hang so cua RoasterUI.h (C_*, F_*), khong dinh nghia them.
 *  - Tat: ENABLE_ROAST_GRAPH 0 trong LyGia_Roaster.ino -> trang chinh tro ve bo cuc cu (cum 6 nut moc).
 */
#pragma once
#include "RoasterGraphData.h"

#define PAGE_GRAPH 2                       // so trang (cung day voi PAGE_MAIN / PAGE_SETTINGS)

// Thang do do thi
#define GR_TMIN   60.0f                    // truc trai: BT (do C)
#define GR_TMAX   240.0f
#define GR_RMAX   25.0f                    // truc phai: RoR 0..25 do C/phut (nhan 10, 20)
#define GR_PTS_MAX 440                     // so diem toi da tren moi duong (= chieu rong vung ve; man ngang phong to ~404)
#define GR_EV_TP   100                     // chi so moc ao cho TP trong GR_EV_IDX (khong phai nut bam)
#define GR_DOTS    6                       // so cham moc: Charge, DE, 1FC, 2FC, Drop + TP (turning point, tu nhan)
#define GR_LINE_BT_W  2                    // do day net duong BT (px)
#define GR_LINE_ROR_W 1                    // do day net duong RoR (px)
#define GR_DOT_SIZE   9                    // duong kinh cham moc (px)
#define GR_PAN 1                          // 1 = keo ngang tren do thi de xem lai khi me rang qua 12 phut (khong keo = tu truot theo). 0 = chi tu truot
#define GR_REF_ROR 0                       // 1 = ve them duong RoR mau cua profile. 0 = chi ve duong BT mau (v2.7.4: bo RoR mau cho do roi)
#define GR_TP_OFS_PX 24                    // nhan TP "TP 98.9°" (1 dong): cach cham 24 px sang phai va 24 px xuong (= 2 lan co chu 12 px), net cheo manh noi ve cham
#include "lv_font_ly_12.h"                 // font 12 px cho nhiet do moc (khong can bat LV_FONT_MONTSERRAT_12 trong lv_conf.h)
#define F_TINY (&lv_font_ly_12)
#include "lv_font_ly_36.h"                 // v2.7.5: so BT / RoR man DOC - cao dong dung 40 px = bang nut phong to
#include "lv_font_ly_32.h"                 // v2.7.5: so BT / RoR man NGANG (cot phai hep hon)
#define F_VAL   (&lv_font_ly_36)           // man doc:  "225.4" rong 95 px
#define F_VALL  (&lv_font_ly_32)           // man ngang: "225.4" rong 82 px
#define GR_COL_R_L 174                     // NGANG: be rong cot phai TRANG GRAPH (UI_COL_R 142 khong du cho so to)
#define GR_MARK_LABEL_TOP 1                // 1 = nhan moc (DE, FCs, SCs, DR) dat sat mep tren khung do thi, noi xuong cham bang net manh -> khong che duong BT / RoR. 0 = nhan ngay tren cham nhu cu

static bool g_graph_built = false;
static bool g_gr_zoom = false;                                        // phong to bieu do (chi sau Drop): an nut moc, Heater, Air (+ bang so lieu o man ngang)
static lv_obj_t *gr_zoom_btn = nullptr;
static lv_point_precise_t gr_zoom_pts[4][3];
static bool gr_zoom_land = false;                                      // nut o tieu de (ngang: an/hien) hay hang so lieu (doc: mo/sang)
static int c_gr_zoom_vis = -1;
static int gr_nb = 0, gr_nr = 0, gr_nrb = 0;
static lv_obj_t *gr_tp_lead = nullptr;                                // net cheo noi nhan TP -> cham
static lv_point_precise_t gr_tp_lead_pts[2];                         // so diem dang ve cua duong BT / RoR / BT mau (de nhan moc tranh duong ve)
static CtlRow gr_heat = {}, gr_air = {};
static int  c_gr3_pid = -1;

static lv_obj_t *gr_btn_start, *gr_lb_start;
static lv_obj_t *gr_ev_lb[GR_DOTS];
static lv_obj_t *gr_ev_tp[GR_DOTS];                                  // nhiet do BT duoi ten moc
static lv_obj_t *gr_ev_ln[GR_DOTS];                                  // net manh noi nhan moc xuong cham (GR_MARK_LABEL_TOP)                                  // chu viet tat canh cham moc
static const char *const GR_EV_ABBR[GR_DOTS] = { "CH", "DE", "FCs", "SCs", "DR", "TP" };   // viet tat chuan quoc te
static lv_obj_t *gr_bt, *gr_ror, *gr_time;
#define GR_COL_DEV 0xA9643B                // mau pha Development (nau); Dry = xanh la, Maillard = cam
static lv_obj_t *gr_dtr;                                              // DTR: thoi gian phat trien (tu 1FC)
static lv_obj_t *gr_dtr2;                                             // DTR: do tang nhiet tu 1FC (tach rieng tu v2.7.5)
static lv_obj_t *gr_bar, *gr_seg[3], *gr_seg_lb[3];                  // thanh ty le Dry / Maillard / Development
static int gr_bar_w = 0;
static int c_g_bar = -1;
static lv_obj_t *gr_card, *gr_plot, *gr_line_bt, *gr_line_ror;
static lv_obj_t *gr_grid[3], *gr_yl[3], *gr_rl[3], *gr_tl[10];
static lv_obj_t *gr_ev_dot[GR_DOTS];
static int gr_px, gr_py, gr_pw, gr_ph;     // vung ve (toa do trong the do thi)
static lv_point_precise_t gr_pts_bt[GR_PTS_MAX], gr_pts_ror[GR_PTS_MAX];
static lv_point_precise_t gr_pts_grid[3][2];
#if ENABLE_PROFILE
#define GR_REF_BT_COL  0xC2570C            // BT mau: cam DAM (toi hon duong BT that FF8F1F)
#define GR_REF_ROR_COL 0x1F6FBF            // RoR mau: xanh duong DAM (toi hon duong RoR that 38BDF8)
#define GR_REF_BT_OPA  LV_OPA_COVER           // duong mau: cung mau nhung SANG / NHAT hon 1 tone so voi duong that (tron them GR_REF_LIGHTEN / 255 mau trang)
#define GR_REF_ROR_OPA LV_OPA_COVER
#define GR_REF_LIGHTEN 100
static lv_obj_t *gr_line_rbt, *gr_line_rror, *gr_ref_dot[3];       // duong mau BT / RoR + 3 cham moc de nghi (TP, DE, FCs)
#define GR_NAME_PAUSE_MS 1500                                          // ten profile dai: dung o moi dau 1.5 s roi chay tiep
static lv_obj_t *gr_prof_name;                                       // ten profile dang chon: ngay duoi dong ho Time o tieu de
static int gr_time_y0 = 0;                                           // y dong ho khi khong co ten profile (canh giua nut)       // duong mau BT / RoR + 3 cham moc de nghi (TP, DE, FCs)
static lv_point_precise_t gr_pts_rbt[GR_PTS_MAX], gr_pts_rror[GR_PTS_MAX];
#endif
static uint32_t c_g_ev = 0xFFFFFFFFu;
static int c_g_cnt = -1, c_g_axis = -1, c_g_unit = -1, c_g_rok = -1;
// Cua so thoi gian truot: rong GR_WIN giay; qua 12 phut thi cua so tu truot theo (giu le phai 30 s), co the keo ngang de xem lai (GR_PAN)
#define GR_WIN 720
static int gr_win = GR_WIN;             // cua so dang ve (giay). Phong to man doc: nho hon de giu nguyen ti le truc (keo ngang de xem)
static int   gr_t0 = 0;                 // giay bat dau cua cua so
static bool  gr_manual = false;         // true: nguoi dung dang keo xem (khong tu truot); cham 1 lan vao do thi de ve lai che do tu truot
static float gr_rmax = GR_RMAX;         // thang RoR: 25, tu nang len khi RoR trong cua so > 25 (toi da 60)
static int   gr_gridR[3] = { 10, 20, 30 };

#if GR_PAN
// Keo ngang tren do thi de xem lai khi me rang dai hon 12 phut; cham 1 lan (khong keo) de ve che do tu truot theo thoi gian
static bool  gr_dragged = false;
static float gr_acc = 0.0f;
static void gr_plot_ev(lv_event_t *e) {
    const lv_event_code_t c = lv_event_get_code(e);
    const int tmax = (int)gCount + 30 > gr_win ? (int)gCount + 30 - gr_win : 0;
    if (c == LV_EVENT_PRESSED) { gr_dragged = false; gr_acc = 0.0f; }
    else if (c == LV_EVENT_PRESSING) {
        lv_indev_t *in = lv_indev_active();
        if (!in || tmax <= 0) return;
        lv_point_t v; lv_indev_get_vect(in, &v);
        if (v.x == 0) return;
        gr_acc += (float)v.x;
        if (!gr_dragged && fabsf(gr_acc) < 4.0f) return;
        gr_dragged = true; gr_manual = true;
        gr_t0 -= (int)lroundf((float)v.x * (float)gr_win / (float)(gr_pw - 1));
        if (gr_t0 < 0) gr_t0 = 0;
        if (gr_t0 >= tmax) { gr_t0 = tmax; gr_manual = false; }
        c_g_cnt = -1;                                                  // ve lai
    } else if (c == LV_EVENT_RELEASED) {
        if (!gr_dragged && gr_manual) { gr_manual = false; c_g_cnt = -1; }
    }
}
#endif
static const int      GR_EV_IDX[GR_DOTS] = { EV_CHARGE, EV_DE, EV_FC1, EV_FC2, EV_DROP, GR_EV_TP };   // cac moc ve thanh cham tren duong BT
// Moc "TP" khong phai nut bam: tu nhan tu gTpIdx (RoasterGraphData.h); cac moc con lai lay tu Snap
static inline bool     gr_ev_has(const Snap &s, int e)  { return e == GR_EV_TP ? (gTpIdx >= 0 && gTpIdx < (int)gCount) : ((s.evDone & (1 << e)) && s.evMs[e] != EV_NO_TIME); }
static inline uint32_t gr_ev_ms(const Snap &s, int e)   { return e == GR_EV_TP ? (uint32_t)gTpIdx * 1000u : s.evMs[e]; }
static inline float    gr_ev_temp(const Snap &s, int e) { return e == GR_EV_TP ? gBT[gTpIdx < 0 ? 0 : gTpIdx] / 10.0f : s.evTemp[e]; }

static inline int gr_y_bt(float c) {
    float k = constrain((c - GR_TMIN) / (GR_TMAX - GR_TMIN), 0.0f, 1.0f);
    return (int)lroundf((1.0f - k) * (float)(gr_ph - 1));
}
static inline int gr_y_ror(float r) {
    float k = constrain(r / gr_rmax, 0.0f, 1.0f);
    return (int)lroundf((1.0f - k) * (float)(gr_ph - 1));
}
static inline int gr_x(float sec, int axis) { return (int)lroundf((sec - (float)gr_t0) * (float)(gr_pw - 1) / (float)axis); }

// -----------------------------------------------------------------------------
// Nut "Roast Graph" tren trang chinh (thay cum 6 nut moc)
// -----------------------------------------------------------------------------
static void ev_open_graph(lv_event_t *) { ui_request_page(PAGE_GRAPH); }

// Nut Graph (bo cuc moi): o vuong dung canh the nhiet do - hinh bieu do nho + chu "Graph >"
static void build_graph_button_v3(int x, int y, int w, int h) {
    static lv_point_precise_t p_bt[6]  = { {3, 30}, {10, 22}, {16, 25}, {24, 14}, {32, 16}, {44, 4} };
    static lv_point_precise_t p_ror[4] = { {3, 33}, {14, 10}, {24, 22}, {44, 27} };
    lv_obj_t *b = lv_button_create(lv_screen_active());
    lv_obj_remove_style_all(b);
    lv_obj_set_pos(b, x, y);
    lv_obj_set_size(b, w, h);
    lv_obj_set_style_bg_color(b, lv_color_hex(C_CARD), 0);
    lv_obj_set_style_bg_opa(b, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(b, lv_color_hex(C_BTN), LV_STATE_PRESSED);
    lv_obj_set_style_radius(b, UI_RADIUS, 0);
    lv_obj_set_style_border_width(b, 1, 0);
    lv_obj_set_style_border_color(b, lv_color_hex(C_ORANGE), 0);
    lv_obj_remove_flag(b, LV_OBJ_FLAG_SCROLLABLE);
    const bool horiz = (w > h + 20);                           // o nam ngang (trang NGANG): hinh | chu tren 1 hang
    const int ix = horiz ? (w - 48 - 8 - 66) / 2 : (w - 48) / 2;
    const int iy = horiz ? (h - 36) / 2 : h / 2 - 40;
    lv_obj_t *l1 = lv_line_create(b);
    lv_line_set_points(l1, p_ror, 4);
    lv_obj_set_pos(l1, ix, iy);
    lv_obj_set_style_line_color(l1, lv_color_hex(C_BLUE), 0);
    lv_obj_set_style_line_width(l1, 2, 0);
    lv_obj_set_style_line_rounded(l1, true, 0);
    lv_obj_t *l2 = lv_line_create(b);
    lv_line_set_points(l2, p_bt, 6);
    lv_obj_set_pos(l2, ix, iy);
    lv_obj_set_style_line_color(l2, lv_color_hex(C_ORANGE), 0);
    lv_obj_set_style_line_width(l2, 3, 0);
    lv_obj_set_style_line_rounded(l2, true, 0);
    static char gbuf[24]; snprintf(gbuf, sizeof gbuf, "%s " LV_SYMBOL_RIGHT, tr("Graph"));
    lv_obj_t *t = make_label(b, gbuf, fnt(F_NORM), C_ORANGE);
    if (horiz) lv_obj_align(t, LV_ALIGN_LEFT_MID, ix + 48 + 8, 0);
    else       lv_obj_align(t, LV_ALIGN_CENTER, 0, 14);
    lv_obj_add_event_cb(b, ev_open_graph, LV_EVENT_CLICKED, NULL);
}

// -----------------------------------------------------------------------------
// Thanh phan trang bieu do
// -----------------------------------------------------------------------------
static lv_obj_t *gr_make_line(lv_obj_t *parent, uint32_t col, int width, int w, int h) {
    lv_obj_t *l = lv_line_create(parent);
    lv_obj_set_pos(l, 0, 0);
    lv_obj_set_size(l, w, h);
    lv_obj_set_style_line_color(l, lv_color_hex(col), 0);
    lv_obj_set_style_line_width(l, width, 0);
    lv_obj_set_style_line_rounded(l, true, 0);
    lv_obj_add_flag(l, LV_OBJ_FLAG_HIDDEN);
    return l;
}

// Bam nut phong to: doi trang thai roi dung lai trang Graph (lv_async_call trong ui_request_page)
static void gr_zoom_click(lv_event_t *) { g_gr_zoom = !g_gr_zoom; gr_manual = false; ui_request_page(PAGE_GRAPH); }

// Nut phong to / thu nho: co bang nut Setting (UI_BTN), bieu tuong 2 mui ten cheo (kieu HiBean): phong to = chi ra 2 goc, thu nho = chi vao giua.
// Man doc: nam cung hang bang BT / RoR / DTR (ben phai), luon hien nhung MO + khong bam duoc cho toi khi Drop.
// Man ngang: nam ben trai nut Setting o tieu de, AN cho toi khi Drop. END (xoa moc Drop) -> tu thu nho + an / mo lai (graph_tick).
static void gr_build_zoom_btn(int x, int y, bool land) {
    gr_zoom_land = land;
    gr_zoom_btn = v3_button(lv_screen_active(), x, y, UI_BTN, UI_BTN, C_BTN, C_BORDER, "", F_SMALL, C_CREAM);
    lv_obj_add_event_cb(gr_zoom_btn, gr_zoom_click, LV_EVENT_CLICKED, nullptr);
    const int A = 11, Z = UI_BTN - 12, M1 = 17, M2 = UI_BTN - 18, hk = 6;    // A / Z: 2 goc cua bieu tuong; M1 / M2: gan giua
    const bool in = g_gr_zoom;
    const int tTRx = in ? M2 : Z, tTRy = in ? M1 : A, eTRx = in ? Z : M2, eTRy = in ? A : M1;   // t = dau mui ten, e = duoi
    const int tBLx = in ? M1 : A, tBLy = in ? M2 : Z, eBLx = in ? A : M1, eBLy = in ? Z : M2;
    const int sg = in ? 1 : -1;
    int k = 0;
    auto put = [&](int x0p, int y0p, int x1p, int y1p, int x2p, int y2p, int n) {
        gr_zoom_pts[k][0].x = x0p; gr_zoom_pts[k][0].y = y0p;
        gr_zoom_pts[k][1].x = x1p; gr_zoom_pts[k][1].y = y1p;
        gr_zoom_pts[k][2].x = x2p; gr_zoom_pts[k][2].y = y2p;
        lv_obj_t *ln = lv_line_create(gr_zoom_btn);
        lv_line_set_points(ln, gr_zoom_pts[k], n);
        lv_obj_set_style_line_color(ln, lv_color_hex(C_CREAM), 0);
        lv_obj_set_style_line_width(ln, 2, 0);
        lv_obj_set_style_line_rounded(ln, true, 0);
        lv_obj_remove_flag(ln, LV_OBJ_FLAG_CLICKABLE);
        k++;
    };
    put(eTRx, eTRy, tTRx, tTRy, 0, 0, 2);                                  // than tren-phai
    put(tTRx + sg * hk, tTRy, tTRx, tTRy, tTRx, tTRy - sg * hk, 3);        // dau tren-phai
    put(eBLx, eBLy, tBLx, tBLy, 0, 0, 2);                                  // than duoi-trai
    put(tBLx - sg * hk, tBLy, tBLx, tBLy, tBLx, tBLy + sg * hk, 3);        // dau duoi-trai
    if (land) lv_obj_add_flag(gr_zoom_btn, LV_OBJ_FLAG_HIDDEN);
    else { lv_obj_add_state(gr_zoom_btn, LV_STATE_DISABLED); lv_obj_set_style_opa(gr_zoom_btn, LV_OPA_40, 0); }
    c_gr_zoom_vis = -1;
}

// Tieu de trang bieu do (bo cuc moi): < | dong ho | [Phong to: chi man ngang] | Setting | START/END
static void gr_header_v3(int W) {
    lv_obj_t *scr = lv_screen_active();
    const int by = UI_MARGIN;
    lv_obj_t *bk = v3_button(scr, UI_MARGIN, by, UI_BTN, UI_BTN, C_BTN, C_BORDER, LV_SYMBOL_LEFT, F_MED, C_CREAM);
    lv_obj_add_event_cb(bk, [](lv_event_t *) { ui_request_page(PAGE_MAIN); }, LV_EVENT_CLICKED, NULL);
    const int sx = W - UI_MARGIN - UI_START_W;
    gr_btn_start = v3_button(scr, sx, by, UI_START_W, UI_BTN, C_GREEN, C_GREEN, tr("START"), fnt(F_MED), C_BG, &gr_lb_start);
    lv_obj_add_event_cb(gr_btn_start, ev_start, LV_EVENT_CLICKED, NULL);
    int gr = sx;
#if ENABLE_SETTINGS_PAGE
    gr = sx - UI_GAP - UI_BTN;
    v3_settings_button(scr, gr, by);
#endif
    if (ui_landscape()) { gr = gr - UI_GAP - UI_BTN; gr_build_zoom_btn(gr, by, true); }   // man ngang: nut phong to ben trai Setting (an toi khi Drop)
    const int gl = UI_MARGIN + UI_BTN;
    gr_time = make_label(scr, "00:00", F_BIG, C_CREAM);       // dong ho canh giua khoang trong
    lv_obj_set_width(gr_time, gr - gl);
    lv_obj_set_style_text_align(gr_time, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_pos(gr_time, gl, by + (UI_BTN - (int)lv_font_get_line_height(F_BIG)) / 2);
#if ENABLE_PROFILE
    gr_time_y0 = by + (UI_BTN - (int)lv_font_get_line_height(F_BIG)) / 2;
    gr_prof_name = make_label(scr, "", F_SMALL, C_MUTED);                    // ten profile: ngay duoi dong ho (an khi rang Manual)
    lv_obj_set_width(gr_prof_name, gr - gl);
    {   // ten dai hon cho trong: chay ngang toi cuoi ten, dung GR_NAME_PAUSE_MS, chay nguoc ve dau, dung, lap lai (ten ngan: dung yen o giua)
        static lv_anim_t name_anim;
        lv_anim_init(&name_anim);
        lv_anim_set_repeat_count(&name_anim, LV_ANIM_REPEAT_INFINITE);
        lv_anim_set_playback_delay(&name_anim, GR_NAME_PAUSE_MS);
        lv_anim_set_repeat_delay(&name_anim, GR_NAME_PAUSE_MS);
        lv_obj_set_style_anim(gr_prof_name, &name_anim, 0);
        lv_label_set_long_mode(gr_prof_name, LV_LABEL_LONG_SCROLL);
    }
    lv_obj_set_style_text_align(gr_prof_name, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_pos(gr_prof_name, gl, UI_HEADER_H - 3 - (int)lv_font_get_line_height(F_SMALL));
    lv_obj_add_flag(gr_prof_name, LV_OBJ_FLAG_HIDDEN);
#endif
    v3_header_line(W);
}

// Luoi so lieu (doc, khong khung): hang 1 = BT | RoR, hang 2 = DTR (thoi gian phat trien - nhiet tang tu 1FC). Nhan canh trai, so canh phai.
// Nhan so can GIUA trong o rong w (v2.7.5: hang so lieu khong nhan, khong khung, nen can giua cho can doi)
static lv_obj_t *gr_val_mid(lv_obj_t *parent, const char *txt, const lv_font_t *f, uint32_t col, int x, int y, int w) {
    lv_obj_t *v = make_label(parent, txt, f, col);
    lv_obj_set_width(v, w);
    lv_label_set_long_mode(v, LV_LABEL_LONG_CLIP);
    lv_obj_set_style_text_align(v, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_pos(v, x, y);
    return v;
}

static lv_obj_t *gr_val_right(lv_obj_t *parent, const char *txt, const lv_font_t *f, uint32_t col, int x, int y, int w) {
    lv_obj_t *v = make_label(parent, txt, f, col);
    lv_obj_set_width(v, w);
    lv_label_set_long_mode(v, LV_LABEL_LONG_CLIP);
    lv_obj_set_style_text_align(v, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_pos(v, x, y);
    return v;
}
// MAN DOC - hang so lieu (v2.7.5): BT | RoR | DTR | nut phong to, tat ca tren MOT hang cao GR_INFO_H (40 px).
// Chi con con SO: khong nhan "BT/RoR/DTR", khong don vi, khong khung bao. Phan biet bang mau: BT cam, RoR xanh, DTR xam.
// Be rong 254 px (320 - 2*10 le - 6 khe - 40 nut): BT 102 | RoR 88 | DTR 52. Chuoi dai nhat: BT "225.4" = 95 px, RoR "-12.8" = 79 px (font 36).
// DTR 2 dong font 16 (cao 18): thoi gian tren, do tang nhiet duoi -> 36 px, can giua trong 40 px.
static void gr_info_grid(int x, int y, int wAll = 0) {
    lv_obj_t *scr = lv_screen_active();
    const lv_font_t *fd = fnt(F_NORM);
    const int lv_ = (int)lv_font_get_line_height(F_VAL), ld = (int)lv_font_get_line_height(fd);
    const int CW = wAll > 0 ? wAll : ui_w() - 2 * x;
    const int wDtr = 52, rest = CW - wDtr - 2 * UI_GAP, wBt = rest * 54 / 100, wRor = rest - wBt;
    gr_bt  = gr_val_mid(scr, "--.-", F_VAL, C_ORANGE, x, y, wBt);
    gr_ror = gr_val_mid(scr, "--",   F_VAL, C_BLUE,   x + wBt + UI_GAP, y, wRor);
    const int xd = x + wBt + wRor + 2 * UI_GAP, yd = y + (lv_ - 2 * ld) / 2;
    gr_dtr  = gr_val_mid(scr, "--:--", fd, C_MUTED, xd, yd, wDtr);
    gr_dtr2 = gr_val_mid(scr, "--",    fd, C_MUTED, xd, yd + ld, wDtr);
}
// Khung so lieu (NGANG, bo cuc moi): BT / RoR (20 px) + nhan DTR va gia tri DTR (16 px) o dong rieng
// MAN NGANG - the so lieu cot phai (v2.7.5): BT | RoR chung mot dong (nhu man doc),
// DTR o dong duoi cung dang NAM NGANG: thoi gian ben trai, do tang nhiet ben phai. Khong nhan, khong don vi.
// Cot phai rong GR_COL_R_L (174) thay vi UI_COL_R (142): can 82 + 6 + 72 = 160 px long trong cho "225.4" va "-12.8" o font 32.
static void gr_info_card3(int x, int y, int w, int h) {
    lv_obj_t *c = make_card(x, y, w, h);
    lv_obj_set_style_radius(c, UI_RADIUS, 0);
    const lv_font_t *fd = fnt(F_MED);
    const int lv_ = (int)lv_font_get_line_height(F_VALL), ld = (int)lv_font_get_line_height(fd);
    const int iw = w - 2 * UI_PAD_X, half = (iw - UI_GAP) / 2, x2 = UI_PAD_X + half + UI_GAP, w2 = iw - half - UI_GAP;
    const int used = lv_ + 10 + ld;
    int yy = (h - used) / 2; if (yy < UI_PAD_Y) yy = UI_PAD_Y;
    gr_bt  = gr_val_mid(c, "--.-", F_VALL, C_ORANGE, UI_PAD_X, yy, half);
    gr_ror = gr_val_mid(c, "--",   F_VALL, C_BLUE,   x2,        yy, w2);
    yy += lv_ + 10;
    gr_dtr  = gr_val_mid(c, "--:--", fd, C_MUTED, UI_PAD_X, yy, half);
    gr_dtr2 = gr_val_mid(c, "--",    fd, C_MUTED, x2,        yy, w2);
}

// Thanh ty le 3 pha: Dry (xanh la) | Maillard (cam) | Development (nau)
static void gr_build_phase_bar(int x, int y, int w, int h) {
    gr_bar = lv_obj_create(lv_screen_active());
    lv_obj_remove_style_all(gr_bar);
    lv_obj_set_pos(gr_bar, x, y);
    lv_obj_set_size(gr_bar, w, h);
    lv_obj_set_style_radius(gr_bar, 5, 0);
    lv_obj_set_style_clip_corner(gr_bar, true, 0);
    lv_obj_set_style_bg_color(gr_bar, lv_color_hex(C_TRACK), 0);
    lv_obj_set_style_bg_opa(gr_bar, LV_OPA_COVER, 0);
    lv_obj_remove_flag(gr_bar, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_remove_flag(gr_bar, LV_OBJ_FLAG_CLICKABLE);
    gr_bar_w = w;
    const uint32_t col[3] = { C_GREEN, C_ORANGE, GR_COL_DEV };
    const uint32_t tcol[3] = { C_BG, C_BG, C_CREAM };
    for (int i = 0; i < 3; i++) {
        gr_seg[i] = lv_obj_create(gr_bar);
        lv_obj_remove_style_all(gr_seg[i]);
        lv_obj_set_pos(gr_seg[i], 0, 0);
        lv_obj_set_size(gr_seg[i], 0, h);
        lv_obj_set_style_bg_color(gr_seg[i], lv_color_hex(col[i]), 0);
        lv_obj_set_style_bg_opa(gr_seg[i], LV_OPA_COVER, 0);
        lv_obj_remove_flag(gr_seg[i], LV_OBJ_FLAG_CLICKABLE);
        gr_seg_lb[i] = make_label(gr_seg[i], "", F_SMALL, tcol[i]);
        lv_label_set_long_mode(gr_seg_lb[i], LV_LABEL_LONG_CLIP);
        lv_obj_set_style_text_align(gr_seg_lb[i], LV_TEXT_ALIGN_CENTER, 0);
    }
    c_g_bar = -1;
}

// The do thi: nen, luoi, nhan truc, cac duong, vach moc, TP
static void gr_build_chart(int x, int y, int w, int h) {
    gr_card = make_card(x, y, w, h);
    lv_obj_set_style_radius(gr_card, 12, 0);
    gr_px = 30; gr_py = 6;
    gr_pw = w - gr_px - 26; gr_ph = h - gr_py - 20;
    if (gr_pw > GR_PTS_MAX) gr_pw = GR_PTS_MAX;

    gr_plot = lv_obj_create(gr_card);
    lv_obj_remove_style_all(gr_plot);
    lv_obj_set_pos(gr_plot, gr_px, gr_py);
    lv_obj_set_size(gr_plot, gr_pw, gr_ph);
    lv_obj_remove_flag(gr_plot, LV_OBJ_FLAG_SCROLLABLE);
#if GR_PAN
    lv_obj_add_flag(gr_plot, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(gr_plot, gr_plot_ev, LV_EVENT_ALL, nullptr);
#else
    lv_obj_remove_flag(gr_plot, LV_OBJ_FLAG_CLICKABLE);
#endif

    static const int gridC[3] = { 100, 150, 200 };       // luoi + nhan truc BT
    static const int gridR[3] = { 10, 20, 30 };           // nhan truc RoR
    for (int i = 0; i < 3; i++) {
        gr_pts_grid[i][0].x = 0;           gr_pts_grid[i][0].y = gr_y_bt((float)gridC[i]);
        gr_pts_grid[i][1].x = gr_pw - 1;   gr_pts_grid[i][1].y = gr_y_bt((float)gridC[i]);
        gr_grid[i] = gr_make_line(gr_plot, C_BORDER, 1, gr_pw, gr_ph);
        lv_line_set_points(gr_grid[i], gr_pts_grid[i], 2);
        lv_obj_remove_flag(gr_grid[i], LV_OBJ_FLAG_HIDDEN);

        gr_yl[i] = make_label(gr_card, "", F_SMALL, C_MUTED);
        lv_obj_set_width(gr_yl[i], gr_px - 2);
        lv_obj_set_style_text_align(gr_yl[i], LV_TEXT_ALIGN_RIGHT, 0);
        lv_obj_set_pos(gr_yl[i], 0, gr_py + gr_y_bt((float)gridC[i]) - 8);

        gr_rl[i] = make_label(gr_card, "", F_SMALL, C_BLUE);
        lv_obj_set_pos(gr_rl[i], gr_px + gr_pw + 3, gr_py + gr_y_ror((float)gridR[i]) - 8);
    }
    for (int i = 0; i < 10; i++) {
        gr_tl[i] = make_label(gr_card, "", F_SMALL, C_MUTED);
        lv_obj_set_width(gr_tl[i], 36);
        lv_obj_set_style_text_align(gr_tl[i], LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_add_flag(gr_tl[i], LV_OBJ_FLAG_HIDDEN);
    }
#if ENABLE_PROFILE
    gr_line_rror = gr_make_line(gr_plot, C_BLUE,   1, gr_pw, gr_ph);          // duong mau nam duoi duong that
    gr_line_rbt  = gr_make_line(gr_plot, C_ORANGE, 1, gr_pw, gr_ph);
    lv_obj_set_style_line_color(gr_line_rror, lv_color_hex(GR_REF_ROR_COL), 0);
    lv_obj_set_style_line_color(gr_line_rbt,  lv_color_hex(GR_REF_BT_COL), 0);
    lv_obj_set_style_line_opa(gr_line_rror, GR_REF_ROR_OPA, 0);
    lv_obj_set_style_line_opa(gr_line_rbt,  GR_REF_BT_OPA, 0);
    lv_obj_set_style_line_dash_width(gr_line_rror, 4, 0); lv_obj_set_style_line_dash_gap(gr_line_rror, 3, 0);
    lv_obj_set_style_line_dash_width(gr_line_rbt,  4, 0); lv_obj_set_style_line_dash_gap(gr_line_rbt,  3, 0);
    for (int i = 0; i < 3; i++) {                                              // vong rong, mo: moc de nghi cua profile
        lv_obj_t *d = lv_obj_create(gr_plot);
        lv_obj_remove_style_all(d);
        lv_obj_set_size(d, 7, 7);
        lv_obj_set_style_radius(d, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_border_width(d, 1, 0);
        lv_obj_set_style_border_color(d, lv_color_hex(C_GREEN), 0);
        lv_obj_set_style_border_opa(d, LV_OPA_70, 0);
        lv_obj_remove_flag(d, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_flag(d, LV_OBJ_FLAG_HIDDEN);
        gr_ref_dot[i] = d;
    }
#endif
    gr_line_ror = gr_make_line(gr_plot, C_BLUE,   GR_LINE_ROR_W, gr_pw, gr_ph);
    gr_line_bt  = gr_make_line(gr_plot, C_ORANGE, GR_LINE_BT_W,  gr_pw, gr_ph);
    for (int i = 0; i < GR_DOTS; i++) {                   // cham moc (nam tren duong BT)
        lv_obj_t *d = lv_obj_create(gr_card);
        lv_obj_remove_style_all(d);
        lv_obj_set_size(d, GR_DOT_SIZE, GR_DOT_SIZE);
        lv_obj_set_style_radius(d, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(d, lv_color_hex(GR_EV_IDX[i] == EV_DROP ? C_RED : C_GREEN), 0);
        lv_obj_set_style_bg_opa(d, LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(d, 1, 0);              // vien trang mong cho de thay tren duong cong
        lv_obj_set_style_border_color(d, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_border_opa(d, LV_OPA_COVER, 0);
        lv_obj_remove_flag(d, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_flag(d, LV_OBJ_FLAG_HIDDEN);
        gr_ev_dot[i] = d;
        lv_obj_t *lb = make_label(gr_card, GR_EV_ABBR[i], F_TINY, GR_EV_IDX[i] == EV_DROP ? C_RED : C_GREEN);    // chu viet tat canh cham (12 px)
        lv_obj_set_width(lb, 52);
        lv_label_set_long_mode(lb, LV_LABEL_LONG_CLIP);
        lv_obj_set_style_text_align(lb, i == 0 ? LV_TEXT_ALIGN_LEFT : i == 2 ? LV_TEXT_ALIGN_RIGHT : LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_style_bg_color(lb, lv_color_hex(C_CARD), 0);
        lv_obj_set_style_bg_opa(lb, LV_OPA_60, 0);
        lv_obj_add_flag(lb, LV_OBJ_FLAG_HIDDEN);
        gr_ev_lb[i] = lb;
        lv_obj_t *tp = make_label(gr_card, "", F_TINY, C_CREAM);                // nhiet do BT luc bam moc (font 12 px)
        lv_obj_set_width(tp, 52);
        lv_label_set_long_mode(tp, LV_LABEL_LONG_CLIP);
        lv_obj_set_style_text_align(tp, i == 0 ? LV_TEXT_ALIGN_LEFT : i == 2 ? LV_TEXT_ALIGN_RIGHT : LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_style_bg_color(tp, lv_color_hex(C_CARD), 0);
        lv_obj_set_style_bg_opa(tp, LV_OPA_60, 0);
        lv_obj_add_flag(tp, LV_OBJ_FLAG_HIDDEN);
        gr_ev_tp[i] = tp;
        lv_obj_t *ln = lv_obj_create(gr_card);                 // net noi nhan -> cham
        lv_obj_remove_style_all(ln);
        lv_obj_set_style_bg_color(ln, lv_color_hex(GR_EV_IDX[i] == EV_DROP ? C_RED : C_GREEN), 0);
        lv_obj_set_style_bg_opa(ln, LV_OPA_50, 0);
        lv_obj_remove_flag(ln, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_flag(ln, LV_OBJ_FLAG_HIDDEN);
        gr_ev_ln[i] = ln;
    }
    gr_tp_lead = lv_line_create(gr_card);
    lv_obj_set_style_line_color(gr_tp_lead, lv_color_hex(C_GREEN), 0);
    lv_obj_set_style_line_opa(gr_tp_lead, LV_OPA_50, 0);
    lv_obj_set_style_line_width(gr_tp_lead, 1, 0);
    lv_obj_remove_flag(gr_tp_lead, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(gr_tp_lead, LV_OBJ_FLAG_HIDDEN);

    c_g_cnt = -1; c_g_ev = 0xFFFFFFFFu; c_g_axis = -1; c_g_unit = -1;
}

// -----------------------------------------------------------------------------
// Dung trang bieu do (goi tu ui_rebuild_cb sau lv_obj_clean)
// -----------------------------------------------------------------------------
static void ui_build_graph() {
    gr_zoom_btn = nullptr;
    const bool L = ui_landscape();
    const int W = ui_w();
    gr_heat = CtlRow{}; gr_air = CtlRow{}; c_gr3_pid = -1;
    gr_bt = gr_ror = gr_dtr = gr_dtr2 = nullptr;                                    // phong to man ngang: khong co bang so lieu
    gr_win = GR_WIN;
    const int CW = W - 2 * UI_MARGIN, H = ui_h();
    gr_header_v3(W);
    c_g_rok = -1;
    c_g_bar = -1;
    const int barH = 18, yBarZ = H - UI_MARGIN - barH;                    // phong to: thanh 3 pha sat day man hinh
    if (!L) {
        // DOC: tieu de | BT RoR / DTR + nut phong to | bieu do | thanh 3 pha | 6 nut moc | Heater | Air   (phong to: chi bo 6 nut moc)
        const int yInfo = UI_HEADER_H + UI_GAP;
        gr_info_grid(UI_MARGIN, yInfo, CW - UI_GAP - UI_BTN);
        gr_build_zoom_btn(W - UI_MARGIN - UI_BTN, yInfo + 2, false);
        const int yChart = yInfo + 40 + UI_GAP;      // hang so lieu cao dung 40 px (= nut phong to) tu v2.7.5
        const int yAir = H - UI_MARGIN - UI_ROW_H, yHeat = yAir - UI_GAP - UI_ROW_H;
        const int evH = 2 * UI_BTN + UI_GAP, yEv = yHeat - UI_GAP - evH;
        const int yBarN = yEv - UI_GAP - barH, yBarZ2 = yHeat - UI_GAP - barH;
        const int hN = yBarN - UI_GAP - yChart, hZ = yBarZ2 - UI_GAP - yChart;
        if (g_gr_zoom) gr_win = (int)lroundf((float)GR_WIN * (float)(hN - 26) / (float)(hZ - 26));   // giu nguyen ti le truc (26 = le tren + duoi vung ve): keo ngang de xem
        const int yBar = g_gr_zoom ? yBarZ2 : yBarN;
        gr_build_chart(UI_MARGIN, yChart, CW, g_gr_zoom ? hZ : hN);
        lv_obj_set_style_radius(gr_card, UI_RADIUS, 0);
        gr_build_phase_bar(UI_MARGIN, yBar, CW, barH);
        if (!g_gr_zoom) build_event_cluster(UI_MARGIN, yEv, CW, evH, 3, UI_GAP, UI_RADIUS);
        gr_heat = v3_ctl_row(UI_MARGIN, yHeat, CW, true);
        gr_air  = v3_ctl_row(UI_MARGIN, yAir,  CW, false);
    } else if (g_gr_zoom) {
        // NGANG phong to: bieu do + thanh 3 pha rong het man hinh (bo bang so lieu, 6 nut moc, Heater, Air)
        const int y0 = UI_HEADER_H + UI_GAP;
        gr_build_chart(UI_MARGIN, y0, CW, yBarZ - UI_GAP - y0);
        lv_obj_set_style_radius(gr_card, UI_RADIUS, 0);
        gr_build_phase_bar(UI_MARGIN, yBarZ, CW, barH);
    } else {
        // NGANG: cot trai (bieu do + thanh 3 pha) | cot phai UI_COL_R (thong so + 6 nut moc 3 x 2) ; hang duoi: Heater | Air (gon)
        const int RW = GR_COL_R_L, LW = CW - UI_GAP - RW, RX = UI_MARGIN + LW + UI_GAP;   // cot phai rong hon de chua so to
        const int y0 = UI_HEADER_H + UI_GAP;
        const int yRow = H - UI_MARGIN - UI_ROW_H;
        const int yBar = yRow - UI_GAP - barH;
        const int hChart = yBar - UI_GAP - y0;
        gr_build_chart(UI_MARGIN, y0, LW, hChart);
        lv_obj_set_style_radius(gr_card, UI_RADIUS, 0);
        gr_build_phase_bar(UI_MARGIN, yBar, LW, barH);
        const int evH = 2 * UI_BTN + UI_GAP, yEv = yBar + barH - evH;      // day cum nut thang day thanh 3 pha
        gr_info_card3(RX, y0, RW, yEv - UI_GAP - y0);
        build_event_cluster(RX, yEv, RW, evH, 3, UI_GAP, UI_RADIUS);
        const int hw = (CW - UI_GAP) / 2;
        gr_heat = v3_ctl_row(UI_MARGIN, yRow, hw, true, true);
        gr_air  = v3_ctl_row(UI_MARGIN + hw + UI_GAP, yRow, CW - hw - UI_GAP, false, true);
    }
    g_graph_built = true;
}

// -----------------------------------------------------------------------------
// Cap nhat do thi tu du lieu (chi khi co thay doi)
// -----------------------------------------------------------------------------
static void gr_refresh_plot(const Snap &s) {
    const int cnt = gCount;
    const int axis = gr_win;                               // cua so 12 phut (phong to man doc: ngan hon); qua cua so thi truot (gr_t0)
    const int tmax = cnt + 30 > gr_win ? cnt + 30 - gr_win : 0;     // vi tri truot lon nhat (giu le phai 30 s)
    if (cnt == 0) gr_manual = false;                       // me moi: ve che do tu truot
    if (!gr_manual || gr_t0 > tmax) gr_t0 = tmax;
    if (gr_t0 < 0) gr_t0 = 0;
    // thang RoR: nang len khi RoR trong cua so > 25
    float rmx = 0.0f;
    for (int i = gr_t0; i < cnt && i <= gr_t0 + axis; i++) if (gROR[i] != GRAPH_NO_ROR && i >= gTpIdx && gTpIdx >= 0 && gROR[i] / 10.0f > rmx) rmx = gROR[i] / 10.0f;
    float nr_ = GR_RMAX;
    if (rmx > GR_RMAX) { nr_ = ceilf(rmx / 5.0f) * 5.0f; if (nr_ > 60.0f) nr_ = 60.0f; }
    gr_rmax = nr_;
    const int rstep = gr_rmax > 30.0f ? 20 : 10;
    for (int i = 0; i < 3; i++) gr_gridR[i] = (i + 1) * rstep;
    const int unit = g_set.unit;
    uint32_t evKey = s.evDone;                              // doi khi co moc moi hoac doi gio moc
    for (int i = 0; i < EV_COUNT; i++) evKey = evKey * 31u + s.evMs[i] / 1000u;
#if ENABLE_PROFILE
    evKey = evKey * 31u + (uint32_t)(g_prof_sel + 1);
#endif
    const int viewKey = gr_t0 * 64 + (int)gr_rmax;                  // doi khi cua so truot hoac doi thang RoR
    if (cnt == c_g_cnt && evKey == c_g_ev && viewKey == c_g_axis && unit == c_g_unit) return;
    c_g_cnt = cnt; c_g_ev = evKey; c_g_axis = viewKey; c_g_unit = unit;
    char buf[32];

    // nhan truc BT / RoR (doi don vi neu can)
    static const int gridC[3] = { 100, 150, 200 };
    for (int i = 0; i < 3; i++) {
        snprintf(buf, sizeof(buf), "%d", (int)lroundf(to_disp((float)gridC[i])));
        set_text_if_changed(gr_yl[i], buf);
        if ((float)gr_gridR[i] > gr_rmax) { lv_obj_add_flag(gr_rl[i], LV_OBJ_FLAG_HIDDEN); }
        else {
            snprintf(buf, sizeof(buf), "%d", (int)lroundf(unit == UNIT_F ? gr_gridR[i] * 1.8f : (float)gr_gridR[i]));
            set_text_if_changed(gr_rl[i], buf);
            lv_obj_set_pos(gr_rl[i], gr_px + gr_pw + 3, gr_py + gr_y_ror((float)gr_gridR[i]) - 8);
            lv_obj_remove_flag(gr_rl[i], LV_OBJ_FLAG_HIDDEN);
        }
    }
    // nhan truc thoi gian: so phut nguyen, cach 2 phut, chi nhung moc nam trong cua so
    const int step = axis <= 420 ? 60 : 120;
    int nl = 0;
    for (int sec = ((gr_t0 + step - 1) / step) * step; sec <= gr_t0 + axis && nl < 10; sec += step, nl++) {
        snprintf(buf, sizeof(buf), "%d", sec / 60);
        set_text_if_changed(gr_tl[nl], buf);
        lv_obj_set_pos(gr_tl[nl], gr_px + gr_x((float)sec, axis) - 18, gr_py + gr_ph + 2);
        lv_obj_remove_flag(gr_tl[nl], LV_OBJ_FLAG_HIDDEN);
    }
    for (int i = nl; i < 10; i++) lv_obj_add_flag(gr_tl[i], LV_OBJ_FLAG_HIDDEN);
    const int tpIdx = gTpIdx;                               // TP: xem RoasterGraphData.h. Duong RoR chi ve tu TP tro di
    // duong BT va RoR: lay mau theo cot diem anh
    int nb = 0, nr = 0;
    for (int xi = 0; xi < gr_pw && xi < GR_PTS_MAX; xi++) {
        const int idx = gr_t0 + (int)((long)xi * axis / (gr_pw - 1));
        if (idx >= cnt) break;
        gr_pts_bt[nb].x = xi; gr_pts_bt[nb].y = gr_y_bt(gBT[idx] / 10.0f); nb++;
        if (tpIdx >= 0 && idx >= tpIdx && gROR[idx] != GRAPH_NO_ROR) { gr_pts_ror[nr].x = xi; gr_pts_ror[nr].y = gr_y_ror(gROR[idx] / 10.0f); nr++; }
    }
    if (nb >= 2) { lv_line_set_points(gr_line_bt, gr_pts_bt, nb);   lv_obj_remove_flag(gr_line_bt, LV_OBJ_FLAG_HIDDEN); }
    else lv_obj_add_flag(gr_line_bt, LV_OBJ_FLAG_HIDDEN);
    if (nr >= 2) { lv_line_set_points(gr_line_ror, gr_pts_ror, nr); lv_obj_remove_flag(gr_line_ror, LV_OBJ_FLAG_HIDDEN); }
    else lv_obj_add_flag(gr_line_ror, LV_OBJ_FLAG_HIDDEN);
    gr_nb = nb; gr_nr = nr; gr_nrb = 0;

#if ENABLE_PROFILE
    // duong mau cua profile da chon: BT va RoR theo giay ke tu Charge (cung truc voi duong that)
    int rb = 0, rr = 0;
    if (g_prof_sel >= 0 && g_prof_sel < PROFILE_COUNT) {
        const ProfileDef &pf = PROFILE_FACTORY[g_prof_sel];
        for (int xi = 0; xi < gr_pw && xi < GR_PTS_MAX; xi++) {
            const float sec = (float)gr_t0 + (float)xi * (float)axis / (float)(gr_pw - 1);
            const float fi = sec / (float)pf.refStep;
            const int i0 = (int)fi;
            if (i0 >= (int)pf.refN - 1) break;
            const float fr = fi - (float)i0;
            const float bt = ((float)pf.refBT[i0] + ((float)pf.refBT[i0 + 1] - (float)pf.refBT[i0]) * fr) / 10.0f;
            gr_pts_rbt[rb].x = xi; gr_pts_rbt[rb].y = gr_y_bt(bt); rb++;
            const int16_t r0 = pf.refRoR[i0], r1 = pf.refRoR[i0 + 1];
            if (GR_REF_ROR && sec >= (float)pf.tp.t && r0 != GRAPH_NO_ROR && r1 != GRAPH_NO_ROR) {                  // RoR mau ve tu moc TP cua profile
                gr_pts_rror[rr].x = xi; gr_pts_rror[rr].y = gr_y_ror(((float)r0 + (float)(r1 - r0) * fr) / 10.0f); rr++;
            }
        }
        const ProfMark mk[3] = { pf.tp, pf.de, pf.fc };
        for (int i = 0; i < 3; i++) {
            if (mk[i].bt10 > 0 && mk[i].t >= gr_t0 && mk[i].t < gr_t0 + axis) {          // bt10 = 0: profile khong co moc de nghi
                lv_obj_set_pos(gr_ref_dot[i], gr_x((float)mk[i].t, axis) - 3, gr_y_bt(mk[i].bt10 / 10.0f) - 3);
                lv_obj_remove_flag(gr_ref_dot[i], LV_OBJ_FLAG_HIDDEN);
            } else lv_obj_add_flag(gr_ref_dot[i], LV_OBJ_FLAG_HIDDEN);
        }
    } else for (int i = 0; i < 3; i++) lv_obj_add_flag(gr_ref_dot[i], LV_OBJ_FLAG_HIDDEN);
    if (g_prof_sel >= 0 && g_prof_sel < PROFILE_COUNT) {
        set_text_if_changed(gr_prof_name, PROFILE_FACTORY[g_prof_sel].name);
        lv_obj_remove_flag(gr_prof_name, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_y(gr_time, UI_MARGIN - 4);                                // dong ho nhuong cho dong ten profile ben duoi
    } else { lv_obj_add_flag(gr_prof_name, LV_OBJ_FLAG_HIDDEN); lv_obj_set_y(gr_time, gr_time_y0); }
    if (rb >= 2) { lv_line_set_points(gr_line_rbt, gr_pts_rbt, rb);    lv_obj_remove_flag(gr_line_rbt, LV_OBJ_FLAG_HIDDEN); }
    else lv_obj_add_flag(gr_line_rbt, LV_OBJ_FLAG_HIDDEN);
    if (rr >= 2) { lv_line_set_points(gr_line_rror, gr_pts_rror, rr);  lv_obj_remove_flag(gr_line_rror, LV_OBJ_FLAG_HIDDEN); }
    else lv_obj_add_flag(gr_line_rror, LV_OBJ_FLAG_HIDDEN);
    gr_nrb = rb;
#endif

    // ---- Nhan moc: ten (14 px) + nhiet do (12 px) ----
    // Ten dat tren cham (~3 chieu cao chu), net manh noi xuong cham; rieng TP dat DUOI BEN PHAI cham (cach GR_TP_GAP_LINES chieu cao chu).
    // Nhiet do: uu tien ngay DUOI ten; neu che duong BT / RoR / BT mau ma dat BEN PHAI ten khong che thi dat ben phai.
    // Thu tu uu tien dat: FCs, CH, TP, DE, DR, SCs; nhan sau tranh nhan truoc (doi phia, ha xuong, hoac bo nhiet do).
    const int lhP = (int)lv_font_get_line_height(F_TINY), lhT = (int)lv_font_get_line_height(F_TINY);   // ten moc va nhiet do deu 12 px
    const int cwC = lv_obj_get_width(gr_card), plotB = gr_py + gr_ph;
    const bool wantT = (ENABLE_EVENT_TEMP != 0);
    int bx0[GR_DOTS], by0[GR_DOTS], bx1[GR_DOTS], by1[GR_DOTS], tmode[GR_DOTS], nameW[GR_DOTS], tempW[GR_DOTS], sideOf[GR_DOTS];
    bool placed[GR_DOTS] = {};
    char tbuf[GR_DOTS][16];
    auto ovLabels = [&](int x0, int y0, int x1, int y1) -> int {
        int ov = 0;
        for (int j = 0; j < GR_DOTS; j++) if (placed[j] && x0 < bx1[j] && x1 > bx0[j] && y0 < by1[j] && y1 > by0[j])
            ov += (x1 < bx1[j] ? x1 : bx1[j]) - (x0 > bx0[j] ? x0 : bx0[j]) + 1;
        return ov;
    };
    auto segHit = [&](const lv_point_precise_t *pt, int n, int x0, int y0, int x1, int y1) -> bool {
        for (int k = 0; k + 1 < n; k++) {
            const int ax = (int)pt[k].x + gr_px, bx = (int)pt[k + 1].x + gr_px;
            if ((ax < x0 && bx < x0) || (ax > x1 && bx > x1)) continue;
            int ya = (int)pt[k].y + gr_py, yb = (int)pt[k + 1].y + gr_py;
            if (ya > yb) { const int t = ya; ya = yb; yb = t; }
            if (yb >= y0 && ya <= y1) return true;
        }
        return false;
    };
    auto curveHit = [&](int x0, int y0, int x1, int y1) -> bool {
        bool h = segHit(gr_pts_bt, gr_nb, x0, y0, x1, y1) || segHit(gr_pts_ror, gr_nr, x0, y0, x1, y1);
#if ENABLE_PROFILE
        h = h || segHit(gr_pts_rbt, gr_nrb, x0, y0, x1, y1);
#endif
        return h;
    };
    auto inCard = [&](int &x0, int w) { if (x0 < 0) x0 = 0; if (x0 + w > cwC) x0 = cwC - w; };
    static const int ORDER[GR_DOTS] = { 2, 0, 5, 1, 4, 3 };
    static const int PREF[GR_DOTS][3] = { {0, 1, 2}, {1, 2, 0}, {2, 1, 0}, {1, 0, 2}, {1, 2, 0}, {0, 1, 2} };
    for (int oi = 0; oi < GR_DOTS; oi++) {
        const int i = ORDER[oi], e = GR_EV_IDX[i];
        if (!(cnt > 0 && gr_ev_has(s, e) && (int)(gr_ev_ms(s, e) / 1000) >= gr_t0 && (int)(gr_ev_ms(s, e) / 1000) <= gr_t0 + axis)) continue;
        const int sec = (int)(gr_ev_ms(s, e) / 1000), idx = sec < cnt ? sec : cnt - 1;
        const int x = gr_px + gr_x(gr_ev_ms(s, e) / 1000.0f, axis), y = gr_py + gr_y_bt(gBT[idx] / 10.0f);
        snprintf(tbuf[i], sizeof tbuf[i], "%.1f°", to_disp(gr_ev_temp(s, e)));
        lv_point_t a, b;
        lv_text_get_size(&a, GR_EV_ABBR[i], F_TINY, 0, 0, 200, LV_TEXT_FLAG_NONE);
        lv_text_get_size(&b, tbuf[i], F_TINY, 0, 0, 200, LV_TEXT_FLAG_NONE);
        nameW[i] = a.x; tempW[i] = b.x;
        const int wB = (wantT && b.x > a.x ? b.x : a.x) + 2, hB = lhP + (wantT ? lhT : 0);   // khoi "nhiet do duoi ten"
        const int wR = a.x + (wantT ? 3 + b.x : 0) + 2;                                          // khoi "nhiet do ben phai ten"
        int tm = wantT ? 0 : 2, side = PREF[i][0], x0, ly, w = wB, h = hB;
        if (i == 5) {                                                                // TP: 1 dong "TP 98.9°", cach cham GR_TP_OFS_PX sang phai + xuong
            side = 0; tm = wantT ? 1 : 2; w = wR; h = lhP;
            x0 = x + GR_TP_OFS_PX; ly = y + GR_TP_OFS_PX - lhP / 2;
            if (ly + h > plotB) ly = plotB - h;                                     // khong du cho phia duoi: nang len cho vua khung
            if (ly < 1) ly = 1;
            inCard(x0, w);
            tmode[i] = tm; sideOf[i] = side;
            bx0[i] = x0; by0[i] = ly; bx1[i] = x0 + w; by1[i] = ly + h; placed[i] = true;
            continue;
        } else {
            const int gapTop = y - GR_DOT_SIZE / 2;
            ly = gapTop - 3 * lhP; if (ly < 1) ly = 1;
            if (gapTop - (ly + hB) < 0) {                                            // cham sat mep tren: khong dat tren duoc
                // thu ngang cham (phai, roi trai) - lay cho khong che duong ve / nhan khac; khong co thi dat duoi cham
                const int my = y - lhP / 2 < 1 ? 1 : y - lhP / 2;
                const int cand[2] = { x + GR_DOT_SIZE / 2 + 3, x - GR_DOT_SIZE / 2 - 3 - wB };
                int pick = -1;
                for (int c = 0; c < 2 && pick < 0; c++) {
                    const int cx0 = cand[c];
                    if (cx0 < 0 || cx0 + wB > cwC || my + hB > plotB + 2) continue;
                    if (!curveHit(cx0, my, cx0 + wB, my + hB) && ovLabels(cx0, my, cx0 + wB, my + hB) == 0) pick = c;
                }
                if (pick >= 0) {
                    side = pick == 0 ? 0 : 2;
                    tmode[i] = wantT ? 0 : 2; sideOf[i] = side;
                    bx0[i] = cand[pick]; by0[i] = my; bx1[i] = cand[pick] + wB; by1[i] = my + hB; placed[i] = true;
                    continue;
                }
                ly = y + GR_DOT_SIZE / 2 + 2;
            }
            int bestOv = 1 << 30;
            for (int c = 0; c < 3; c++) {
                const int sd = PREF[i][c];
                int cx0 = sd == 0 ? x + 3 : sd == 2 ? x - 3 - wB : x - wB / 2;
                int ov = 0;
                if (cx0 < 0) ov += -cx0 * 4;
                if (cx0 + wB > cwC) ov += (cx0 + wB - cwC) * 4;
                ov += ovLabels(cx0, ly, cx0 + wB, ly + hB);
                if (ov < bestOv) { bestOv = ov; side = sd; }
                if (ov == 0) break;
            }
            x0 = side == 0 ? x + 3 : side == 2 ? x - 3 - wB : x - wB / 2;
            inCard(x0, wB);
            if (ovLabels(x0, ly, x0 + wB, ly + hB) > 0) {                          // van de nhan khac: ha xuong duoi nhan do (neu con cho truoc cham)
                int ly2 = ly;
                for (int j = 0; j < GR_DOTS; j++) if (placed[j] && x0 < bx1[j] && x0 + wB > bx0[j] && ly2 < by1[j] && ly2 + hB > by0[j]) ly2 = by1[j];
                if (ly2 + hB <= gapTop - 2 && ovLabels(x0, ly2, x0 + wB, ly2 + hB) == 0) ly = ly2;
                else if (ly2 + lhP <= gapTop - 2 && ovLabels(x0, ly2, x0 + a.x + 2, ly2 + lhP) == 0) { ly = ly2; tm = 2; h = lhP; w = a.x + 2; }   // chi ghi ten
            }
        }
        // nhiet do che duong ve? thu dat ben phai ten (cung dong)
        if (tm == 0 && curveHit(x0, ly + lhP, x0 + wB, ly + hB)) {
            int rx0 = (i == 5 || side == 0) ? x0 : side == 2 ? x - 3 - wR : x - wR / 2;
            inCard(rx0, wR);
            if (!curveHit(rx0 + a.x + 2, ly, rx0 + wR, ly + lhP) && ovLabels(rx0, ly, rx0 + wR, ly + lhP) == 0) { tm = 1; x0 = rx0; w = wR; h = lhP; }
        }
        tmode[i] = tm; sideOf[i] = side;
        bx0[i] = x0; by0[i] = ly; bx1[i] = x0 + w; by1[i] = ly + h; placed[i] = true;
    }

    // cham moc (nam tren duong BT) + nhan + net noi
    for (int i = 0; i < GR_DOTS; i++) {
        const int e = GR_EV_IDX[i];
        if (!placed[i]) { if (i == 5 && gr_tp_lead) lv_obj_add_flag(gr_tp_lead, LV_OBJ_FLAG_HIDDEN); lv_obj_add_flag(gr_ev_dot[i], LV_OBJ_FLAG_HIDDEN); lv_obj_add_flag(gr_ev_lb[i], LV_OBJ_FLAG_HIDDEN); lv_obj_add_flag(gr_ev_ln[i], LV_OBJ_FLAG_HIDDEN); lv_obj_add_flag(gr_ev_tp[i], LV_OBJ_FLAG_HIDDEN); continue; }
        const int sec = (int)(gr_ev_ms(s, e) / 1000), idx = sec < cnt ? sec : cnt - 1;
        const int x = gr_px + gr_x(gr_ev_ms(s, e) / 1000.0f, axis), y = gr_py + gr_y_bt(gBT[idx] / 10.0f);
        lv_obj_set_pos(gr_ev_dot[i], x - GR_DOT_SIZE / 2, y - GR_DOT_SIZE / 2);
        lv_obj_remove_flag(gr_ev_dot[i], LV_OBJ_FLAG_HIDDEN);
        // net manh noi nhan <-> cham
        const int dotT = y - GR_DOT_SIZE / 2, dotB = y + GR_DOT_SIZE / 2;
        if (i == 5) {                                                                // TP: net cheo tu cham toi goc tren trai nhan
            lv_obj_add_flag(gr_ev_ln[i], LV_OBJ_FLAG_HIDDEN);
            if (gr_tp_lead) {
                gr_tp_lead_pts[0].x = x + 3; gr_tp_lead_pts[0].y = y + 3;
                gr_tp_lead_pts[1].x = bx0[i] - 2; gr_tp_lead_pts[1].y = by0[i] + lhP / 2;
                lv_line_set_points(gr_tp_lead, gr_tp_lead_pts, 2);
                lv_obj_remove_flag(gr_tp_lead, LV_OBJ_FLAG_HIDDEN);
            }
        } else if (by1[i] <= dotT - 3)      { lv_obj_set_pos(gr_ev_ln[i], x, by1[i]); lv_obj_set_size(gr_ev_ln[i], 1, dotT - by1[i]); lv_obj_remove_flag(gr_ev_ln[i], LV_OBJ_FLAG_HIDDEN); }
        else if (by0[i] >= dotB + 3 && bx0[i] <= x + 4) { lv_obj_set_pos(gr_ev_ln[i], x, dotB); lv_obj_set_size(gr_ev_ln[i], 1, by0[i] + lhP / 2 - dotB); lv_obj_remove_flag(gr_ev_ln[i], LV_OBJ_FLAG_HIDDEN); }
        else lv_obj_add_flag(gr_ev_ln[i], LV_OBJ_FLAG_HIDDEN);
        if (tmode[i] == 1) {                                                         // nhiet do ben phai ten
            lv_obj_set_width(gr_ev_lb[i], nameW[i] + 2);
            lv_obj_set_style_text_align(gr_ev_lb[i], LV_TEXT_ALIGN_LEFT, 0);
            lv_obj_set_pos(gr_ev_lb[i], bx0[i], by0[i]);
            lv_obj_set_width(gr_ev_tp[i], tempW[i] + 2);
            lv_obj_set_style_text_align(gr_ev_tp[i], LV_TEXT_ALIGN_LEFT, 0);
            lv_obj_set_pos(gr_ev_tp[i], bx0[i] + nameW[i] + 3, by0[i] + (lhP - lhT) / 2 + 1);
        } else {                                                                     // nhiet do duoi ten (hoac khong ghi)
            const int w = bx1[i] - bx0[i];
            const lv_text_align_t al = sideOf[i] == 0 ? LV_TEXT_ALIGN_LEFT : sideOf[i] == 2 ? LV_TEXT_ALIGN_RIGHT : LV_TEXT_ALIGN_CENTER;
            lv_obj_set_width(gr_ev_lb[i], w);  lv_obj_set_style_text_align(gr_ev_lb[i], al, 0);
            lv_obj_set_width(gr_ev_tp[i], w);  lv_obj_set_style_text_align(gr_ev_tp[i], al, 0);
            lv_obj_set_pos(gr_ev_lb[i], bx0[i], by0[i]);
            lv_obj_set_pos(gr_ev_tp[i], bx0[i], by0[i] + lhP);
        }
        lv_obj_remove_flag(gr_ev_lb[i], LV_OBJ_FLAG_HIDDEN);
        set_text_if_changed(gr_ev_tp[i], tbuf[i]);
        if (tmode[i] == 2) lv_obj_add_flag(gr_ev_tp[i], LV_OBJ_FLAG_HIDDEN); else lv_obj_remove_flag(gr_ev_tp[i], LV_OBJ_FLAG_HIDDEN);
    }
}

// DTR, nhiet do tang tu 1FC va thanh ty le 3 pha. Sau Drop: giu nguyen (dong ho Time da dung, nhiet lay theo luc Drop).
static void gr_dev_tick(const Snap &s) {
    const bool dropped = (s.evDone & (1 << EV_DROP)) && s.evMs[EV_DROP] != EV_NO_TIME;
    const uint32_t total = dropped ? s.evMs[EV_DROP] : s.batchMs;
    const bool hasDE = (s.evDone & (1 << EV_DE))  && s.evMs[EV_DE]  != EV_NO_TIME;
    const bool hasFC = (s.evDone & (1 << EV_FC1)) && s.evMs[EV_FC1] != EV_NO_TIME && s.evMs[EV_FC1] <= total;
    char buf[40];

    // --- DTR: thoi gian phat trien (tu 1FC) - nhiet do tang tu 1FC (chot lai luc Drop) ---
    const uint32_t dev = hasFC ? (total - s.evMs[EV_FC1]) : 0;
    bool riseOk = false; float rise = 0.0f;
    if (hasFC && gCount > 0) {
        int i0 = (int)(s.evMs[EV_FC1] / 1000); if (i0 > gCount - 1) i0 = gCount - 1;
        float t1 = s.temp;
        if (dropped) { int i1 = (int)(total / 1000); if (i1 > gCount - 1) i1 = gCount - 1; t1 = gBT[i1] / 10.0f; }
        if (dropped || s.tempOk) { rise = t1 - gBT[i0] / 10.0f; if (g_set.unit == UNIT_F) rise *= 1.8f; riseOk = true; }
    }
    char buf2[16];
    if (hasFC) {
        const uint32_t ds = dev / 1000;
        snprintf(buf, sizeof(buf), "%02u:%02u", (unsigned)(ds / 60), (unsigned)(ds % 60));
        if (riseOk) snprintf(buf2, sizeof(buf2), "%.1f°", rise);
        else        snprintf(buf2, sizeof(buf2), "--");
    } else { snprintf(buf, sizeof(buf), "--:--"); snprintf(buf2, sizeof(buf2), "--"); }
    if (gr_dtr) {
        set_text_if_changed(gr_dtr, buf);
        lv_obj_set_style_text_color(gr_dtr, lv_color_hex(hasFC ? C_GREEN : C_MUTED), 0);
    }
    if (gr_dtr2) {
        set_text_if_changed(gr_dtr2, buf2);
        lv_obj_set_style_text_color(gr_dtr2, lv_color_hex(hasFC ? C_GREEN : C_MUTED), 0);
    }
    // --- Thanh ty le 3 pha (tinh tu Charge) ---
    uint32_t dry = 0, mai = 0, dvl = 0;
    if (total > 0) {
        dry = hasDE ? s.evMs[EV_DE] : total;
        if (dry > total) dry = total;
        if (hasDE) { const uint32_t e = hasFC ? s.evMs[EV_FC1] : total; mai = e > dry ? e - dry : 0; }
        if (hasFC) dvl = dev;
    }
    const uint32_t sum = dry + mai + dvl;
    const int pk[3] = { sum ? (int)(dry * 1000ULL / sum) : 0, sum ? (int)(mai * 1000ULL / sum) : 0, sum ? (int)(dvl * 1000ULL / sum) : 0 };
    const int barKey = (pk[0] << 20) ^ (pk[1] << 10) ^ pk[2] ^ (sum ? 1 << 30 : 0);
    if (barKey != c_g_bar) {
        c_g_bar = barKey;
        static const char *const nm[3] = { "Dry", "Mai", "Dev" };
        int x = 0;
        for (int i = 0; i < 3; i++) {
            int w = (i == 2) ? (sum ? gr_bar_w - x : 0) : (int)((long)pk[i] * gr_bar_w / 1000);
            if (w < 0) w = 0;
            lv_obj_set_pos(gr_seg[i], x, 0);
            lv_obj_set_width(gr_seg[i], w);
            if (pk[i] > 0 && w >= 40) {
                if (w >= 82) snprintf(buf, sizeof(buf), "%s %.1f%%", nm[i], pk[i] / 10.0f);
                else         snprintf(buf, sizeof(buf), "%.1f%%", pk[i] / 10.0f);
                lv_label_set_text(gr_seg_lb[i], buf);
                lv_obj_set_width(gr_seg_lb[i], w);
                lv_obj_set_pos(gr_seg_lb[i], 0, (18 - (int)lv_font_get_line_height(F_SMALL)) / 2);
            } else lv_label_set_text(gr_seg_lb[i], "");
            x += w;
        }
    }
}

// Goi 100 ms/lan tu sync_cb (RoasterUI.h)
static void graph_tick(const Snap &s) {
    if (!g_graph_built) return;
    char buf[32];
    const bool dropped = (s.evDone & (1 << EV_DROP)) && s.evMs[EV_DROP] != EV_NO_TIME;
    if (g_gr_zoom && !dropped) { g_gr_zoom = false; g_graph_built = false; ui_request_page(PAGE_GRAPH); return; }   // END / me moi: ve bo cuc thuong
    if ((int)dropped != c_gr_zoom_vis && gr_zoom_btn) {
        c_gr_zoom_vis = dropped;
        if (gr_zoom_land) {                                                 // ngang: an / hien
            if (dropped) lv_obj_remove_flag(gr_zoom_btn, LV_OBJ_FLAG_HIDDEN); else lv_obj_add_flag(gr_zoom_btn, LV_OBJ_FLAG_HIDDEN);
        } else {                                                            // doc: mo + khoa / sang + bam duoc
            if (dropped) { lv_obj_remove_state(gr_zoom_btn, LV_STATE_DISABLED); lv_obj_set_style_opa(gr_zoom_btn, LV_OPA_COVER, 0); }
            else         { lv_obj_add_state(gr_zoom_btn, LV_STATE_DISABLED);    lv_obj_set_style_opa(gr_zoom_btn, LV_OPA_40, 0); }
        }
    }

    apply_start_button(gr_btn_start, gr_lb_start, s);

    if (s.tempOk) snprintf(buf, sizeof(buf), "%.1f", to_disp(constrain(s.temp, 0.0f, (float)TEMP_MAX)));   // v2.7.5: chi con so, khong don vi
    else          snprintf(buf, sizeof(buf), "--.-");
    if (gr_bt) set_text_if_changed(gr_bt, buf);
    const bool rok = ror_visible(s);       // RoR chi hien tu luc Charge va khi >= ROR_MIN_SHOW
    if (rok) snprintf(buf, sizeof(buf), "%.1f", g_set.unit == UNIT_F ? s.ror * 1.8f : s.ror);              // v2.7.5: chi con so, khong don vi
    else     snprintf(buf, sizeof(buf), "--");
    if (gr_ror) {
        set_text_if_changed(gr_ror, buf);
        if ((int)rok != c_g_rok) { c_g_rok = rok; lv_obj_set_style_text_color(gr_ror, lv_color_hex(rok ? C_BLUE : C_MUTED), 0); }
    }
    const uint32_t sec = s.batchMs / 1000;
    snprintf(buf, sizeof(buf), "%02u:%02u", (unsigned)(sec / 60), (unsigned)(sec % 60));
    set_text_if_changed(gr_time, buf);

    if (!g_gr_zoom) ui_apply_events(s);                                   // phong to: khong co cum nut moc
    gr_dev_tick(s);

    // Heater (PID: hien muc Set) / Air
    v3_ctl_apply(gr_heat, s, true, c_gr3_pid);
    v3_ctl_apply(gr_air, s, false, c_gr3_pid);
    gr_refresh_plot(s);
}
