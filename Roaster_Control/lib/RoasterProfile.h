/*
 * GPL v3 - xem LyGia_Roaster.ino
 *
 * PROFILE RANG MAU: kieu du lieu + danh sach 6 profile goc (flash). Bat/tat: ENABLE_PROFILE (LySetting.h).
 * Giai doan 1 (hien tai): danh sach profile cho hop thoai START / Auto.
 * Dong co chay luat, duong mau tren Graph, luu ban sua cua nguoi dung (phan vung 9.9 MB): lam o cac buoc sau.
 */
#pragma once

#if ENABLE_PROFILE

enum ProfTrig { PT_PREHEAT, PT_CHARGE, PT_TP, PT_DE, PT_FC1S, PT_DROP, PT_BT_GT };
enum ProfAct  { PA_NONE, PA_PID_OFF, PA_PID_ON, PA_HEATER, PA_FAN };
struct ProfAction { uint8_t type; int16_t v; };                 // PID_ON: v = nhiet x10; HEATER/FAN: v = %
struct ProfRule   { uint8_t trig; uint8_t afterTP; int16_t val10; uint8_t nact; ProfAction act[3]; };   // BT_GT: val10 = nhiet x10
struct ProfMark   { int16_t t; int16_t bt10; };                 // moc de nghi: giay tu Charge, nhiet x10
struct ProfileDef {
    const char *name;
    const char *weight;                                         // ghi chu khoi luong khuyen nghi
    const char *process;                                        // kieu so che (Washed / Natural / Honey ...)
    const char *level;                                          // muc rang (Light / Medium / Dark ...)
    int16_t charge10;                                           // nhiet Charge x10
    int16_t preheatPid10;                                       // PID luc PreHeat x10
    uint8_t preheatFan;
    uint8_t autoDE, autoFC;                                     // tu danh dau DE / FC1s (TP luon tu nhan)
    ProfMark tp, de, fc;
    uint8_t refStep;                                            // buoc mau cua duong mau (giay)
    uint16_t refN;
    const int16_t *refBT;                                       // nhiet x10
    const int16_t *refRoR;                                      // RoR x10, -32768 = chua co
    uint8_t nRules;
    const ProfRule *rules;
};

#include "RoasterProfileData.h"


#endif
