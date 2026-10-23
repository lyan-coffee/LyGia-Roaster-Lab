/*
 * GPL v3 - xem LyGia_Roaster.ino
 *
 * LySetting.h - CAU HINH FIRMWARE LY GIA ROASTER
 *
 *  - Moi thong so o day chi khai bao MOT LAN (khong khai bao lai trong lib/). Sua o day la du.
 *  - Giao dien, gia tri mac dinh cua trang Setting va man hinh cho: lib/UIConfig.h (duoc include o cuoi file nay).
 *  - Cac thong so khac van nam tai cho (chan cam ung, buoc +/-, PID, truc bieu do, gia lap...): xem CLAUDE.md muc 6.
 *  - Sau khi sua: build va nap lai firmware.
 */
#pragma once

#define CFG_STR_(x) #x
#define CFG_STR(x)  CFG_STR_(x)          // bien so thanh chuoi: CFG_STR(10) -> "10"

// =============================================================================
// 1. NHAN DIEN THIET BI
// =============================================================================
#define FW_VERSION "v2.7.5"
#define BLE_NAME     "LyGia Roaster"  // ten thiet bi khi quet Bluetooth (BLE). Toi da 26 ky tu (dai hon bi cat)
#define DEVICE_NAME  "LyGia Roaster"             // khong dau (app HiBean hien: DEVICE_NAME + " " + FW_VERSION)

// =============================================================================
// 2. CHAN NOI (doi board / doi day thi sua o day)
// =============================================================================
#define ROASTER_TX_PIN        19      // chan phat lenh xuong may rang (-1 = khong dung)
#define ROASTER_RX_PIN        20      // chan nhan nhiet do tu may rang (-1 = khong dung)
#define ROASTER_BOOT_DELAY_MS 3000    // cho nap firmware xong roi moi chiem chan 19/20 (dung chung chan USB)

// Loa canh bao I2S tren bo JC3248W535EN - CHAN GIA DINH, doi chieu so do mach khi gan loa
#define ALARM_I2S_BCLK  42
#define ALARM_I2S_LRCK  2
#define ALARM_I2S_DOUT  41
#define ALARM_BUZZER_PIN -1           // chi dung khi ALARM_AUDIO_MODE = 2 (coi GPIO)

// =============================================================================
// 3. BAT / TAT TINH NANG  (1 = bat, 0 = bo han chuc nang)
// =============================================================================
#define ENABLE_SETTINGS_PAGE  1       // trang Setting (luu vao NVS). 0 = dung co dinh cac gia tri DEFAULT_* trong UIConfig.h
#define ENABLE_OVERHEAT_ALARM 1       // canh bao qua nhiet (man hinh + loa)
#define ENABLE_SIMULATION     1       // gia lap nhiet do khi khong co tin hieu may rang (bat/tat trong Setting)
#define ENABLE_ROAST_GRAPH    1       // trang bieu do rang (BT, RoR, DTR, thanh 3 pha, 6 nut moc)
#define ENABLE_ABOUT_PAGE     1       // trang Gioi thieu (nut "i" canh chu SETTINGS): logo + thong tin lien he. Thong tin sua o muc 7 cuoi file nay
#define ENABLE_START_DIALOG   1       // bam START hien hop thoai Manual / Auto (Manual: START + PR + sang trang Graph; Auto: chon Profile). 0 = START bat dau ngay nhu cu
#define ENABLE_PROFILE        1       // Profile rang mau (6 profile goc trong flash; Auto tren hop thoai START chon profile). 0 = tat
#define ENABLE_PROFILE_AUTO   1       // Auto: chay cac luat (trigger) cua Profile da chon: dat Heater / Air / PID theo nhiet do BT va cac moc. Nguoi dung van chinh tay duoc. 0 = chi ve duong mau
#define ENABLE_AUTO_MARKS     1       // tu danh dau moc DE / FC1s cho Profile (theo cong tac autoDE / autoFC cua tung profile; TP luon tu nhan). 0 = moi moc deu bam tay
#define ENABLE_ROAST_LOG      1       // luu log moi me rang vao phan vung FFat (/logs/0001.json, 0002... tang dan). Chi luu khi du moc Charge, TP, DE, FC1s, Drop. Can ENABLE_ROAST_GRAPH = 1
#define ENABLE_AUTO_NOTICE    1       // bam Auto + chon profile: hien thong bao "PreHeat" (ten profile, nhiet Charge) vai giay tren trang Graph. 0 = khong thong bao
#define ENABLE_EVENT_TEMP     1       // Graph: duoi ten moc (CH, DE, FCs, SCs, DR) ghi nhiet do BT luc bam. 0 = khong ghi
#define ENABLE_ZH_FONT        1       // 1 = dung font tieng Hoa tu tao (lib/lv_font_zh_14/16/20.h). Cach tao: Readme.txt muc A-V11 / tools/make_zh_font.py. 0 = the Chinese bi mo
#define TOUCH_DEBUG           0       // 1 = in toa do cham ra Serial (can SERIAL_DEBUG 1 trong lib/SerialDebug.h)

// =============================================================================
// 4. AN TOAN
//    Quy tac: chi Drop, END, E-STOP moi cat Heater. Cac dieu kien duoi day chi CANH BAO.
//    Dang PID ma mat tin hieu -> chuyen sang tay, GIU nguyen muc Heater.
// =============================================================================
#define SAFETY_INTERLOCK   1          // 1 = canh bao khi Drum OFF / Cool ON / Air thap ma dang cap nhiet
#define MIN_AIR_FOR_HEAT   10         // Air toi thieu (%) khi cap nhiet; cau canh bao tu lay so nay
#define HIBEAN_TIMEOUT_MS  10000      // mat HiBean qua lau (ms) khi HiBean dang dieu khien Heater -> canh bao
#define TEMP_STALE_MS      5000       // mat tin hieu nhiet qua lau (ms) khi dang cap nhiet -> canh bao (0 = tat)
#define TEMP_SIGNAL_OK_MS  3000       // co khung nhiet do trong khoang nay (ms) thi coi la "co tin hieu"
#define DROP_CONFIRM_MS    5000       // bam Drop lan 1 -> cho bam xac nhan trong khoang nay, het han tu huy
#define END_CONFIRM_MS     5000       // bam END giua me (chua Drop) -> cho xac nhan trong khoang nay

// =============================================================================
// 4b. MOC TU DONG DAT HEATER / AIR (bam PR, CH tren man hinh)
// =============================================================================
// Sau nay khi co rang tu dong / rang theo Profile (trigger nhu HiBean) thi Profile se ghi de cac so nay.
// ---- PID (v2.7.2): bo mau + chinh trong trang Setting ----
#define ENABLE_PID_SETTINGS   1       // 1 = trang Setting co the PID (Soft / Balanced / Fast + P1-P3 cua nguoi dung), luu NVS va nap lai khi khoi dong. 0 = bo muc PID khoi trang Setting, dung co dinh bo PID_DEFAULT_* (Fast) o tren. Can ENABLE_SETTINGS_PAGE 1. 0 = chi dung PID mac dinh Kp 9 / Ki 0.3 / Kd 2.5 (SkiPIDConfig.h)
// PID MAC DINH (cung la bo "Fast": tang nhanh + phanh som, kieu P theo SAI SO). Khi ENABLE_PID_SETTINGS 0 thi chi dung bo nay (co dinh); khi 1 thi day la bo luc khoi dong lan dau (chua luu gi)
#define PID_DEFAULT_KP        18.0
#define PID_DEFAULT_KI        0.3
#define PID_DEFAULT_KD        30.0
#define PID_DEFAULT_PMODE     1       // 1 = P theo sai so (P_ON_E), 0 = P theo so do (P_ON_M)
// ---- Ho tro tang nhiet & han che vuot Set (ap cho MOI bo PID, ke ca khi HiBean/Artisan gui Set) ----
#define ENABLE_PID_ASSIST     1       // 1 = bat. 0 = chi PID thuan
#define PID_BOOST_FAR_C       10      // cach Set > so nay (do C): Heater >= PID_BOOST_FAR_PCT
#define PID_BOOST_FAR_PCT     95
#define PID_BOOST_NEAR_C      5       // cach Set > so nay (do C): Heater >= PID_BOOST_NEAR_PCT
#define PID_BOOST_NEAR_PCT    75
#define PID_BOOST_END_C       2       // het "leo" khi cach Set <= so nay (do C); chi leo lai khi cach Set > PID_BOOST_FAR_C
#define PID_LEAD_S            10      // tinh "cach Set" theo BT du kien sau ngan nay giay (BT + RoR*s/60) de phanh som; 0 = chi dung BT hien tai
#define PID_OVER_SOFT_C       1       // BT vuot Set > so nay: Heater khong qua PID_OVER_SOFT_PCT
#define PID_OVER_SOFT_PCT     10
#define PID_OVER_MAX_C        3       // BT vuot Set > so nay: cat Heater (0%) -> vuot toi da khoang 1-3 do C
#define PID_I_BAND_C          8       // chi khi kieu P = theo sai so (Soft / Balanced / Fast): cach Set > so nay (do C) thi xoa tich phan, Heater chay theo Kp, khong vot qua Set
#define PID_I_PRESET          40      // muc Heater (%) nap san cho phan tich phan khi con xa Set (xap xi muc giu nhiet cua may)
#define ENABLE_EVENT_PRESETS  1       // 1 = bam PR / CH tu dat PID, Heater, Air theo cac so duoi day; 0 = chi ghi moc nhu cu
#define PRESET_PR_TEMP_C      180     // PR: bat PID, giu nhiet do nay (do C) ...
#define PRESET_PR_TEMP_F      356     // ... khi dang o don vi do F thi dung so tron nay (356 F = 180 C)
#define PRESET_PR_AIR         10      // PR: Air (%)
#define PRESET_CH_HEATER      70      // CH: tat PID, Heater (%)
#define PRESET_CH_AIR         20      // CH: Air (%)

// =============================================================================
// 5. HIBEAN
// =============================================================================
#define HIBEAN_DEFAULT_ON  0          // 1 = bat quang ba BLE ngay khi khoi dong
#define HIBEAN_AUTOSTART   1          // 1 = HiBean bat Heater/PID thi dong ho me rang tu chay

// =============================================================================
// 6. CANH BAO QUA NHIET
// =============================================================================
// Khoang cho phep chinh muc canh bao trong trang Setting (do C). Muc mac dinh: DEFAULT_WARN_TEMP trong UIConfig.h.
#define WARN_TEMP_MIN   220           // muc thap nhat nguoi dung chon duoc
#define WARN_TEMP_MAX   260           // muc cao nhat nguoi dung chon duoc
#define WARN_TEMP_STEP  5             // moi lan bam - / + doi bao nhieu do
#define ALARM_HYST_C    3             // nhiet phai giam duoi (muc canh bao - so nay) thi canh bao moi tu tat
#define ALARM_AUDIO_MODE 1            // 0 = tat tieng, 1 = loa I2S tren bo, 2 = coi GPIO (ALARM_BUZZER_PIN)
#define ALARM_VOLUME    6000          // am luong loa: 0..32767, tang neu nho

// =============================================================================
// 7. TRANG GIOI THIEU (nut "i" trong trang Setting)
// =============================================================================
// Chuoi KHONG DAU (font chua co chu Viet co dau). De "" thi dong do tu an. Noi dung dai hon man hinh thi KEO LEN / XUONG duoc (nhu dien thoai, khong thanh cuon).
// DANG LA THONG TIN MAU: thay bang thong tin that truoc khi phat hanh.
#define ABOUT_TAGLINE  "LY GIA Touch Control"
#define ABOUT_PHONE    "+8491 8393 391"
#define ABOUT_ZALO     ""                        // chua dung: de rong thi dong an
#define ABOUT_WEB      ""                        // chua co website: de rong thi dong nay tu an
#define ABOUT_FACEBOOK ""                        // chua dung: de rong thi dong an
#define ABOUT_EMAIL    "lycoffee.ai@gmail.com"
#define ABOUT_ADDRESS  "Ho Chi Minh - Viet Nam"

// =============================================================================
#include "lib/UIConfig.h"             // giao dien + mac dinh trang Setting + man hinh cho
