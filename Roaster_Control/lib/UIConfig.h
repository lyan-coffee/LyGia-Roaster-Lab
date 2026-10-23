/*
 * GPL v3 - xem LyGia_Roaster.ino
 *
 * UIConfig.h - CAC THONG SO GIAO DIEN DUNG CHUNG (theo QuyTac_GiaoDien.txt)
 *  Nguoi ke thua: muon chinh mau / co chu / kich thuoc / khoang cach thi sua O DAY, khong sua rai rac trong code.
 *  Moi thay doi quy tac: cap nhat QuyTac_GiaoDien.txt truoc, roi sua so lieu o day.
 *
 *  Duoc include tu LySetting.h (goc sketch). Gom: so lieu giao dien (Phan 1-4), mac dinh trang Setting (Phan 5),
 *  man hinh cho (Phan 6).
 */
#pragma once

// ---------------- PHAN 1. MAU (RGB hex) ----------------
// Nen & khung
#define UI_C_BG        0x14100D   // nen man hinh
#define UI_C_CARD      0x221B16   // nen the
#define UI_C_BTN       0x33281F   // nen nut
#define UI_C_BORDER    0x3A2E26   // vien the / nut, ranh thanh truot
// Chu
#define UI_C_CREAM     0xF3E5D0   // chu chinh, gia tri
#define UI_C_MUTED     0x9C8C7C   // nhan, chu phu, trang thai tat
// Mau theo CHUC NANG (chi dung trong the cua chuc nang do)
#define UI_C_HEATER    0xFF8F1F   // cam
#define UI_C_AIR       0x38BDF8   // xanh duong
#define UI_C_DRUM      0x43D17A   // xanh la
#define UI_C_COOL      0xA5B4FC   // tim bang
// Mau theo TRANG THAI
#define UI_C_OK        0x43D17A   // START, moc da bam, DTR, ket noi Connected
#define UI_C_WARN      0xFF8F1F   // nut moc ke tiep, PID bat
#define UI_C_DANGER    0xFF5252   // END, Drop, canh bao, mat tin hieu
#define UI_C_HIBEAN    0xFFB300   // trang thai Wait
// Mau DU LIEU (bieu do)
#define UI_C_BT        0xFF8F1F
#define UI_C_ROR       0x38BDF8
#define UI_C_PH_DRY    0x43D17A   // pha Dry
#define UI_C_PH_MAI    0xFF8F1F   // pha Maillard
#define UI_C_PH_DEV    0xA9643B   // pha Development

// ---------------- PHAN 2. THANG CHU (Montserrat) ----------------
//  XL 48 = F_HUGE  : so nhiet do lon trang chinh
//  L  28 = F_BIG   : Time & RoR trang chinh, dong ho tieu de trang bieu do
//  M  20 = F_MED   : gia tri Heater/Air, chu nut tieu de, ten may, BT/RoR trang bieu do
//  N  16 = F_NORM  : ON/OFF Drum/Cool, trang thai ket noi, DTR
//  S  14 = F_SMALL : nhan, trang thai may, ten + gio nut moc, truc bieu do, chu thanh 3 pha, chu PID
// (Cac hang F_* khai bao trong RoasterUI.h vi phu thuoc font duoc bat trong lv_conf.h)

// ---------------- PHAN 3. BIEU TUONG, BO GOC, NUT ----------------
#define UI_ICON        34         // co bieu tuong duy nhat (Heater, Air, Drum, Cool, huy hieu HiBean/Artisan)
#define UI_RADIUS      6          // bo goc the + moi nut
#define UI_BTN         40         // nut - / + va nut tieu de (vuong 40 x 40)

// ---------------- PHAN 4. KHOANG CACH & LUOI ----------------
#define UI_MARGIN      10         // le man hinh: TREN, duoi, trai, phai deu nhau
#define UI_GAP         6          // khoang cach giua cac the
#define UI_PAD_X       7          // dem ngang trong the (bieu tuong cach trai 7 px)
#define UI_PAD_Y       6          // dem doc trong the
#define UI_HEADER_H    (UI_MARGIN + UI_BTN + 6)   // duong ke tieu de o y nay (nut tieu de tu y = UI_MARGIN); noi dung bat dau sau them UI_GAP
#define UI_ROW_H       52         // hang dieu khien 1 dong (UI_BTN + 2 * UI_PAD_Y)

// Kich thuoc rieng (bo cuc)
#define UI_START_W     76         // nut START / END (rut ngan tu 92)
#define UI_PID_W       44         // nut PID trong hang Heater
#define UI_GRAPH_W     76         // nut Graph canh the nhiet do (= UI_START_W), cao bang the nhiet do
#define UI_COL_R       142        // NGANG: be rong cot phai (Drum, Cool, Graph, thong so + nut moc trang bieu do)
#define UI_SET_ICON    26         // bieu tuong Setting (banh rang + co le) trong nut 40 x 40
#define UI_SWITCH_W    46         // cong tac Drum / Cool
#define UI_SWITCH_H    24

// ---------------- PHAN 5. GIA TRI MAC DINH TRANG SETTING ----------------
//  Dung khi nap lan dau (chua co gia tri luu trong NVS), hoac khi ENABLE_SETTINGS_PAGE = 0 (LySetting.h).
#define DEFAULT_ORIENTATION  0    // 0 = doc, 1 = ngang
#define DEFAULT_TEMP_UNIT    0    // 0 = do C, 1 = do F
#define DEFAULT_LANGUAGE     0    // 0 = English, 1 = Tieng Viet, 2 = Chinese
#define DEFAULT_WARN_TEMP    240  // muc canh bao qua nhiet (do C), trong khoang WARN_TEMP_MIN..MAX (LySetting.h)
#define DEFAULT_SIMULATION   0    // 1 = tu bat gia lap khi khoi dong neu khong co tin hieu may rang

// ---------------- PHAN 6. MAN HINH CHO (logo + dai sang) ----------------
#define ENABLE_IDLE_SCREEN   1    // 1 = co man hinh cho (bat/tat them trong Setting); 0 = bo han chuc nang
#define DEFAULT_IDLE_SCREEN  1    // trang thai mac dinh cua nut "Man hinh cho" trong Setting: 1 = bat, 0 = tat
#define IDLE_TIMEOUT_MIN     2    // so phut CHUA START + khong cham man hinh thi vao man hinh cho
#define ENABLE_IDLE_DITHER   1    // 1 = nen man hinh cho tu ve voi hieu ung dither (mem, khong soc vach); 0 = nen gradient cua LVGL (co the thay vach tren man 16 bit)
