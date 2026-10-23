/*
 * GPL v3 - xem LyGia_Roaster.ino
 *
 * MODULE MAN HINH CHO (screensaver): logo Ly Gia Roaster tren nen toi, dai sang nghieng 45 do lướt qua logo.
 *
 * Khi nao hien:  Setting "Idle Screen" bat + CHUA START (khong co me dang mo) + Heater dang tat, lien tuc IDLE_TIMEOUT_MIN phut
 *                (mac dinh 2), VA khong cham man hinh trong tung do thoi gian do. Co tin hieu nhiet do that van vao man hinh cho.
 *                Gia lap: IDLE_IN_SIM 1 = gia lap chua START van vao man hinh cho; 0 = gia lap chan man hinh cho (nhu cu).
 * Khi nao tat:   cham man hinh (lan cham dau chi danh thuc, KHONG bam nham nut ben duoi), hoac START / mo me /
 *                bat Heater / bao dong qua nhiet (IDLE_IN_SIM 0: ca khi bat gia lap).
 *
 * Cach ve: ban sao logo 192x192 (RGB565A8) nam trong RAM; moi khung chi tinh lai dai cheo dang sang (khoang 4 ms),
 *          khong dung lop mat na nen khong co vien vuong. Ve tren lv_layer_top() nhu man hinh canh bao.
 * Bat/tat:  nut "Idle Screen" trong trang Setting (luu NVS, mac dinh DEFAULT_IDLE_SCREEN).
 *           ENABLE_IDLE_SCREEN (lib/UIConfig.h) = 0: bo han chuc nang. Thoi gian cho: IDLE_TIMEOUT_MIN (UIConfig.h).
 *           IDLE_IN_SIM: lib/RoasterSim.h.
 * Doi logo: chay lai script sinh lib/RoasterIdleLogo.h (file sinh tu dong, khong sua tay).
 *
 * Phu thuoc (RoasterUI.h khai bao truoc): ui_w/ui_h, g_disp, Snap, realSignalOk, g_simActive, al_state (neu co canh bao)
 */
#pragma once

#if ENABLE_IDLE_SCREEN || ENABLE_ABOUT_PAGE      // trang Gioi thieu dung chung khung ve logo + nen voi man hinh cho

#include "RoasterIdleLogo.h"

#define IDLE_TIMEOUT_MS   ((uint32_t)IDLE_TIMEOUT_MIN * 60000UL)
#define IDLE_FRAME_MS     40                    // ~25 khung/giay
#define IDLE_SWEEP_MS     4500                  // thoi gian mot luot dai sang
#define IDLE_BAND_HALF    83                    // nua be rong dai sang, don vi (x+y) cua anh 192 px (~43% duong cheo)
#define IDLE_BASE_A256    217                   // do dac logo nen: 0.85
#define IDLE_BG_TOP       0x0F1A28
#define IDLE_BG_BOT       0x070B11

static lv_obj_t      *idle_ov        = nullptr;
static lv_obj_t      *idle_img       = nullptr;
static lv_timer_t    *idle_timer     = nullptr;
static uint16_t      *idle_rgb       = nullptr;     // ban sao RGB565 (mat phang mau)
static uint8_t       *idle_alpha     = nullptr;     // mat phang alpha ke sau
static uint8_t       *idle_mem       = nullptr;
static lv_image_dsc_t idle_dsc;
static bool           idle_about     = false;       // true: dang hien TRANG GIOI THIEU (khong tu an theo Heater / START)
#if ENABLE_IDLE_DITHER
static uint16_t      *idle_bg        = nullptr;     // nen man hinh cho RGB565 da dither (W x H)
static lv_image_dsc_t idle_bgdsc;
#endif
static uint32_t       idle_t0        = 0;
static uint32_t       idle_quiet_ms  = 0;
static volatile bool  idle_wake_req  = false;
static uint16_t       idle_lut[65];
#if defined(LV_CACHE_DEF_SIZE) && (LV_CACHE_DEF_SIZE > 0)
#define idle_cache_drop() lv_image_cache_drop(&idle_dsc)   // chi khi LVGL bat bo nho dem anh
#else
#define idle_cache_drop() ((void)0)                        // khong co bo nho dem: khong can (va ham co the khong duoc khai bao)
#endif                 // dai sang: 0..256, giam dan tu giua ra hai ben

static void idle_lut_init() {
    static const float px[5] = {0.f, .25f, .5f, .75f, 1.f};
    static const float py[5] = {1.f, .68f, .28f, .06f, 0.f};
    for (int i = 0; i <= 64; i++) {
        const float u = i / 64.0f;
        float v = 0;
        for (int k = 0; k < 4; k++)
            if (u >= px[k] && u <= px[k + 1]) { v = py[k] + (py[k + 1] - py[k]) * (u - px[k]) / (px[k + 1] - px[k]); break; }
        idle_lut[i] = (uint16_t)(v * 256.0f + 0.5f);
    }
}

// Tinh 1 diem anh: wq = 0..256 do sang cua dai (0 = chi logo nen mo)
static inline void idle_px(int i, int wq) {
    const uint8_t a = IDLE_LOGO_A[i];
    if (a == 0) { idle_rgb[i] = 0; idle_alpha[i] = 0; return; }
    const uint16_t c = IDLE_LOGO_RGB[i];
    int r = ((c >> 11) & 31) * 255 / 31, g = ((c >> 5) & 63) * 255 / 63, b = (c & 31) * 255 / 31;
    const int f = 294 + ((382 * wq) >> 8);          // 1.15 * (1 + 1.3 * w)
    const int add = (36 * wq) >> 8;                 // anh sang am them
    r = (r * f >> 8) + add;                         // anh sang am (vang nhat), khong ra trang
    g = (g * f >> 8) + (add * 3 >> 2);
    b = (b * f >> 8) + (add >> 2);
    if (r > 255) r = 255;
    if (g > 255) g = 255;
    if (b > 255) b = 255;
    idle_rgb[i]   = (uint16_t)(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3));
    idle_alpha[i] = (uint8_t)((a * (IDLE_BASE_A256 + ((256 - IDLE_BASE_A256) * wq >> 8))) >> 8);
}

// Ve lai dai sang quanh vi tri pc (don vi x+y). Chi cac diem trong dai (cong le) duoc tinh lai.
static void idle_render(int pc) {
    const int D = IDLE_LOGO_PX, H = IDLE_BAND_HALF, pad = 8;
    for (int y = 0; y < D; y++) {
        int x0 = pc - H - pad - y, x1 = pc + H + pad - y;
        if (x0 < 0) x0 = 0;
        if (x1 > D - 1) x1 = D - 1;
        for (int x = x0; x <= x1; x++) {
            int dd = (x + y) - pc; if (dd < 0) dd = -dd;
            int wq = 0;
            if (dd < H) wq = idle_lut[dd * 64 / H];
            idle_px(y * D + x, wq);
        }
    }
}

#if ENABLE_IDLE_DITHER
// Nen gradient doc IDLE_BG_TOP -> IDLE_BG_BOT, tinh 8 bit moi kenh roi dither Bayer 4x4 xuong RGB565: man 16 bit chi co ~4-8 muc cho dai mau toi nay
// nen gradient cua LVGL bi thanh vach ngang; dither tron cac muc lai, mat nhin ra mem. Mau dau / cuoi giu nguyen.
static void idle_make_bg(int W, int H) {
    static const uint8_t bayer[4][4] = { { 0, 8, 2,10}, {12, 4,14, 6}, { 3,11, 1, 9}, {15, 7,13, 5} };
    const size_t bytes = (size_t)W * H * 2;
    uint16_t *b = nullptr;
#ifdef MALLOC_CAP_SPIRAM
    b = (uint16_t *)heap_caps_malloc(bytes, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
#endif
    if (!b) b = (uint16_t *)heap_caps_malloc(bytes, MALLOC_CAP_8BIT);
    if (!b) return;                                     // het RAM: giu nen gradient LVGL
    const int r0 = (IDLE_BG_TOP >> 16) & 255, g0 = (IDLE_BG_TOP >> 8) & 255, b0 = IDLE_BG_TOP & 255;
    const int r1 = (IDLE_BG_BOT >> 16) & 255, g1 = (IDLE_BG_BOT >> 8) & 255, b1 = IDLE_BG_BOT & 255;
    for (int y = 0; y < H; y++) {
        const int t = (H > 1) ? (y * 256 * 256 / (H - 1)) : 0;                    // 0..65536
        const int rv = r0 * 256 + (int)(((int64_t)(r1 - r0) * t) >> 8);            // 8 bit . 8 bit phan le
        const int gv = g0 * 256 + (int)(((int64_t)(g1 - g0) * t) >> 8);
        const int bv = b0 * 256 + (int)(((int64_t)(b1 - b0) * t) >> 8);
        for (int x = 0; x < W; x++) {
            const int th = bayer[y & 3][x & 3];                                     // 0..15
            int r5 = (rv + (th * 2048 / 16)) >> 11;   // buoc 8 muc (8*256) chia 16 nguong
            int g6 = (gv + (th * 1024 / 16)) >> 10;  // buoc 4 muc
            int b5 = (bv + (th * 2048 / 16)) >> 11;
            if (r5 > 31) r5 = 31; if (g6 > 63) g6 = 63; if (b5 > 31) b5 = 31;
            b[(size_t)y * W + x] = (uint16_t)((r5 << 11) | (g6 << 5) | b5);
        }
    }
    idle_bg = b;
    memset(&idle_bgdsc, 0, sizeof(idle_bgdsc));
    idle_bgdsc.header.magic  = LV_IMAGE_HEADER_MAGIC;
    idle_bgdsc.header.cf     = LV_COLOR_FORMAT_RGB565;
    idle_bgdsc.header.w      = W;
    idle_bgdsc.header.h      = H;
    idle_bgdsc.header.stride = W * 2;
    idle_bgdsc.data_size     = bytes;
    idle_bgdsc.data          = (const uint8_t *)b;
}
#endif

static void idle_render_base() {
    for (int i = 0; i < IDLE_LOGO_PX * IDLE_LOGO_PX; i++) idle_px(i, 0);
}

static void idle_anim_cb(lv_timer_t *) {
    if (!idle_img) return;
    const int D = IDLE_LOGO_PX, H = IDLE_BAND_HALF;
    const uint32_t ph = (millis() - idle_t0) % IDLE_SWEEP_MS;
    const int span = 2 * D + 2 * H;
    const int pc = -H + (int)((uint64_t)ph * span / IDLE_SWEEP_MS);
    idle_render(pc);
    idle_cache_drop();
    lv_obj_invalidate(idle_img);
}

static void ev_idle_press(lv_event_t *) { idle_wake_req = true; }   // dong that su lam o idle_tick (khong xoa trong su kien)

static void idle_hide() {
    if (idle_timer) { lv_timer_delete(idle_timer); idle_timer = nullptr; }
    if (idle_ov) { lv_obj_delete(idle_ov); idle_ov = nullptr; idle_img = nullptr; }
    idle_about = false;
    if (idle_mem) { idle_cache_drop(); free(idle_mem); idle_mem = nullptr; idle_rgb = nullptr; idle_alpha = nullptr; }
#if ENABLE_IDLE_DITHER
    if (idle_bg) { free(idle_bg); idle_bg = nullptr; }      // (anh nen chi do ov dung, ov da xoa o tren)
#endif
}

#if ENABLE_ABOUT_PAGE
// Noi dung trang Gioi thieu: logo o tren (hoac ben trai khi ngang), duong ke, mo ta, cac dong lien he, phien ban, goi y dong.
static lv_obj_t *about_text(lv_obj_t *p, const char *txt, const lv_font_t *f, uint32_t col, int x, int y, int w) {
    lv_obj_t *l = make_label(p, txt, f, col);
    lv_obj_set_width(l, w);
    lv_label_set_long_mode(l, LV_LABEL_LONG_CLIP);
    lv_obj_set_style_text_align(l, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_pos(l, x, y);
    return l;
}
static void about_build(lv_obj_t *ov, int W, int H, int logo_x, int logo_y) {
    const bool L = W > H;
    struct Row { const char *label; const char *val; };
    const Row all[6] = { { tr("Phone"), ABOUT_PHONE }, { "Zalo", ABOUT_ZALO }, { "Website", ABOUT_WEB },
                         { "Facebook", ABOUT_FACEBOOK }, { "Email", ABOUT_EMAIL }, { tr("Address"), ABOUT_ADDRESS } };
    Row rows[6]; int n = 0;
    for (int i = 0; i < 6; i++) if (all[i].val[0]) rows[n++] = all[i];            // dong rong thi an
    const bool tag = ABOUT_TAGLINE[0] != 0;
    const int footMin = H - UI_MARGIN - 17 - 4 - 17;                               // vi tri dong "Firmware" khi noi dung ngan (dong goi y nam duoi no)
    int ax, aw, y, pitch = 40;
    lv_obj_t *vl = nullptr;
    if (L) {
        // NGANG: logo ben trai | soc cam DOC (phan cach) | noi dung ben phai, can giua theo chieu doc cung logo
        const int sx = logo_x + IDLE_LOGO_PX + 14;                                  // x soc
        ax = sx + 14; aw = W - ax - UI_MARGIN;
        vl = lv_obj_create(ov);
        lv_obj_remove_style_all(vl);
        lv_obj_set_pos(vl, sx, 40);
        lv_obj_set_style_bg_color(vl, lv_color_hex(UI_C_HEATER), 0);                // cam, mo dan xuong duoi (cung kieu duong ke tieu de)
        lv_obj_set_style_bg_grad_color(vl, lv_color_hex(UI_C_BG), 0);
        lv_obj_set_style_bg_grad_dir(vl, LV_GRAD_DIR_VER, 0);
        lv_obj_set_style_bg_opa(vl, LV_OPA_COVER, 0);
        lv_obj_remove_flag(vl, LV_OBJ_FLAG_CLICKABLE);
        const int ch = (tag ? 30 : 0) + n * pitch;                                  // chieu cao khoi noi dung
        y = (H - ch) / 2 - 12;                                                      // lech len mot chut cho can voi logo
        if (y < 20) y = 20;
    } else {
        // DOC: logo tren cung, duong ke ngang, noi dung dan deu den dong Firmware (neu nhieu dong thi trang keo xuong)
        ax = UI_MARGIN; aw = W - 2 * UI_MARGIN;
        y = logo_y + IDLE_LOGO_PX + 12;
        lv_obj_t *line = lv_obj_create(ov);
        lv_obj_remove_style_all(line);
        lv_obj_set_size(line, 200, 2);
        lv_obj_set_pos(line, (W - 200) / 2, y);
        lv_obj_set_style_bg_color(line, lv_color_hex(UI_C_HEATER), 0);
        lv_obj_set_style_bg_grad_color(line, lv_color_hex(UI_C_BG), 0);
        lv_obj_set_style_bg_grad_dir(line, LV_GRAD_DIR_HOR, 0);
        lv_obj_set_style_bg_opa(line, LV_OPA_COVER, 0);
        lv_obj_remove_flag(line, LV_OBJ_FLAG_CLICKABLE);
        y += 14;
        const int avail = footMin - 10 - (y + (tag ? 30 : 0));                      // cho con lai cho cac dong lien he
        if (n > 0) { pitch = avail / n; if (pitch > 52) pitch = 52; if (pitch < 40) pitch = 40; }
    }
    const int yTop = y;
    if (tag) { about_text(ov, ABOUT_TAGLINE, fnt(F_NORM), UI_C_CREAM, ax, y, aw); y += 30; }
    for (int i = 0; i < n; i++) {
        about_text(ov, rows[i].label, fnt(F_SMALL), UI_C_MUTED, ax, y, aw);
        about_text(ov, rows[i].val, fnt(F_NORM), UI_C_CREAM, ax, y + 17, aw);
        y += pitch;
    }
    const int footY = (y + 10 > footMin) ? y + 10 : footMin;                        // noi dung dai: dong Firmware nam sau cung (keo xuong de thay)
    char buf[40];
    snprintf(buf, sizeof(buf), "Firmware v%s", FW_VERSION);
    about_text(ov, buf, fnt(F_SMALL), UI_C_MUTED, ax, footY, aw);
    about_text(ov, tr("Touch to close"), fnt(F_SMALL), 0x5E5144, ax, footY + 21, aw);
    if (vl) {
        int bottom = footY + 21 + 17 + 20;                                           // soc doc keo dai theo noi dung, toi thieu den H - 40
        if (bottom < H - 40) bottom = H - 40;
        lv_obj_set_size(vl, 2, bottom - 80);
    }
    (void)yTop;
}
#endif

static void idle_show(bool about = false) {
    idle_hide();
    const size_t n = (size_t)IDLE_LOGO_PX * IDLE_LOGO_PX;
    uint8_t *m = nullptr;
#ifdef MALLOC_CAP_SPIRAM
    m = (uint8_t *)heap_caps_malloc(n * 3, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
#endif
    if (!m) m = (uint8_t *)heap_caps_malloc(n * 3, MALLOC_CAP_8BIT);
    if (!m) { idle_quiet_ms = millis(); return; }       // het RAM: bo qua, thu lai sau
    idle_mem = m;
    idle_rgb = (uint16_t *)m;
    idle_alpha = m + n * 2;
    idle_lut_init();
    idle_render_base();

    memset(&idle_dsc, 0, sizeof(idle_dsc));
    idle_dsc.header.magic  = LV_IMAGE_HEADER_MAGIC;
    idle_dsc.header.cf     = LV_COLOR_FORMAT_RGB565A8;
    idle_dsc.header.w      = IDLE_LOGO_PX;
    idle_dsc.header.h      = IDLE_LOGO_PX;
    idle_dsc.header.stride = IDLE_LOGO_PX * 2;
    idle_dsc.data_size     = n * 3;
    idle_dsc.data          = m;

    const int W = ui_w(), H = ui_h();
    idle_ov = lv_obj_create(lv_layer_top());
    lv_obj_remove_style_all(idle_ov);
    lv_obj_set_pos(idle_ov, 0, 0);
    lv_obj_set_size(idle_ov, W, H);
    lv_obj_set_style_bg_color(idle_ov, lv_color_hex(IDLE_BG_TOP), 0);
    lv_obj_set_style_bg_grad_color(idle_ov, lv_color_hex(IDLE_BG_BOT), 0);
    lv_obj_set_style_bg_grad_dir(idle_ov, LV_GRAD_DIR_VER, 0);
    lv_obj_set_style_bg_opa(idle_ov, LV_OPA_COVER, 0);
    lv_obj_add_flag(idle_ov, LV_OBJ_FLAG_CLICKABLE);                 // nuot lan cham dau, khong cho xuyen xuong nut
    lv_obj_remove_flag(idle_ov, LV_OBJ_FLAG_SCROLLABLE);
    if (!about) lv_obj_add_event_cb(idle_ov, ev_idle_press, LV_EVENT_PRESSED, NULL);   // man hinh cho: cham la thoat; Gioi thieu: cham (khong keo) moi dong, xem ben duoi

#if ENABLE_IDLE_DITHER
    idle_make_bg(W, H);
    if (idle_bg) {                                                   // nen dither phu len gradient LVGL (gradient van la du phong neu het RAM)
        lv_obj_t *bg = lv_image_create(idle_ov);
        lv_image_set_src(bg, &idle_bgdsc);
        lv_obj_set_pos(bg, 0, 0);
        lv_obj_remove_flag(bg, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_bg_grad_dir(idle_ov, LV_GRAD_DIR_NONE, 0);   // da co anh nen: bo gradient LVGL phia duoi cho nhe
    }
#endif
    idle_img = lv_image_create(idle_ov);
    lv_image_set_src(idle_img, &idle_dsc);
    int lx = (W - IDLE_LOGO_PX) / 2, ly = (H - IDLE_LOGO_PX) / 2;
#if ENABLE_ABOUT_PAGE
    if (about) {                                                     // Gioi thieu: logo len tren (doc) / sang trai (ngang), thong tin o phan con lai
        idle_about = true;
        if (W > H) { lx = 20; } else { ly = 20; }
        // Khung cuon: keo len / xuong bang ngon tay (co da quan tinh), KHONG hien thanh cuon; noi dung vua man hinh thi khong cuon
        lv_obj_t *sc = lv_obj_create(idle_ov);
        lv_obj_remove_style_all(sc);
        lv_obj_set_pos(sc, 0, 0);
        lv_obj_set_size(sc, W, H);
        lv_obj_set_style_pad_bottom(sc, UI_MARGIN, 0);
        lv_obj_add_flag(sc, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_flag(sc, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_scroll_dir(sc, LV_DIR_VER);
        lv_obj_set_scrollbar_mode(sc, LV_SCROLLBAR_MODE_OFF);
        lv_obj_add_event_cb(sc, ev_idle_press, LV_EVENT_CLICKED, NULL);   // CLICKED (khong phai PRESSED): keo trang khong bi tinh la cham de dong
        lv_obj_set_parent(idle_img, sc);
        about_build(sc, W, H, lx, ly);
    }
#else
    (void)about;
#endif
    lv_obj_set_pos(idle_img, lx, ly);
    lv_obj_remove_flag(idle_img, LV_OBJ_FLAG_CLICKABLE);

    idle_wake_req = false;
    idle_t0 = millis();
    idle_timer = lv_timer_create(idle_anim_cb, IDLE_FRAME_MS, NULL);
}

// Goi moi 100 ms tu sync_cb (moi trang)
static void idle_tick(const Snap &s) {
    const uint32_t now = millis();
#if ENABLE_ABOUT_PAGE
    if (idle_ov && idle_about) {                                     // Gioi thieu: chi dong khi cham, hoac khi co canh bao qua nhiet
        bool alarm = false;
#if ENABLE_OVERHEAT_ALARM
        alarm = (al_state == AL_ACTIVE);
#endif
        if (idle_wake_req || alarm) { idle_wake_req = false; idle_hide(); idle_quiet_ms = now; }
        return;
    }
#endif
#if !ENABLE_IDLE_SCREEN
    (void)s;
    return;
#else
    // Chua START (khong co me dang mo) + Heater dang tat -> dem gio; co tin hieu nhiet that van vao man hinh cho
    // IDLE_IN_SIM 1: gia lap ma chua START van vao man hinh cho (chua START thi chua co duong nhiet de theo doi)
#if IDLE_IN_SIM
    bool busy = s.live || s.heat > 0 || !g_set.idle;     // g_set.idle = 0: nguoi dung tat man hinh cho trong Setting
#else
    bool busy = g_simActive || s.live || s.heat > 0 || !g_set.idle;
#endif
#if ENABLE_OVERHEAT_ALARM
    if (al_state == AL_ACTIVE) busy = true;
#endif
    if (busy) {
        idle_quiet_ms = now;
        if (idle_ov) idle_hide();
        return;
    }
    if (idle_ov) {
        if (idle_wake_req) { idle_wake_req = false; idle_hide(); idle_quiet_ms = now; }
        return;
    }
    if (idle_quiet_ms == 0) idle_quiet_ms = now;
    if ((now - idle_quiet_ms) >= IDLE_TIMEOUT_MS && lv_display_get_inactive_time(g_disp) >= IDLE_TIMEOUT_MS)
        idle_show();
#endif
}

#else
static void idle_tick(const Snap &) {}
#endif
