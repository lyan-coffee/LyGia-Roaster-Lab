/*
 * GPL v3 - xem LyGia_Roaster.ino
 *
 * MODULE GIA LAP (Simulation): khi KHONG co tin hieu nhiet do that tu may rang, nguoi dung co the
 * bat gia lap de thu / chinh thong so (Heater, Air, PID, HiBean...).
 *
 *  - Chi chay khi khong co tin hieu that. Tin hieu that xuat hien -> gia lap tu tat, dung nhiet do that.
 *  - Khi gia lap dang chay, firmware KHONG phat khung dieu khien xuong may rang (che do offline).
 *    Luc bat dau gia lap gui mot khung "tat het" de dua may that (neu dang noi) ve trang thai an toan.
 *  - Gia lap chay nhanh SIM_TIME_SCALE (7) lan thoi gian that: preheat 7 phut -> ~1 phut; RoR hien thi quy ve thoi gian that.
 *  - Nhiet do gia lap duoc dua vao PID va tra loi READ cho HiBean nhu nhiet do that.
 *  - Bat/tat: CHI trong trang Setting (nut "Simulation").
 *    Trang thai khong duoc luu (khoi dong lai = tat, tru khi DEFAULT_SIMULATION 1).
 *  - ENABLE_SIMULATION 0: bo hoan toan chuc nang.
 */
#pragma once
#include "SkiCMD.h"

// Man hinh cho khi dang gia lap: 1 = gia lap ma CHUA START (chua co duong nhiet) van vao man hinh cho;
// 0 = dang gia lap thi khong bao gio vao man hinh cho. (Dung trong lib/RoasterIdle.h)
#define IDLE_IN_SIM 1

#if ENABLE_SIMULATION

// Nhiet do moi truong gia lap (do C). Viet Nam thuong 29..32 -> doi so nay cho phu hop thoi tiet
#define SIM_AMBIENT 30.0f

// Gia lap chay NHANH gap SIM_TIME_SCALE lan thuc te (preheat that ~7 phut -> ~1 phut). RoR duoc quy ve thoi gian that
#ifndef SIM_SCALE_DEFAULT
#define SIM_SCALE_DEFAULT 7        // he so mac dinh (doi trong Setting: x1 x3 x5 x7)
#endif
static uint8_t g_simScale = SIM_SCALE_DEFAULT;   // luu trong NVS (RoasterSettings.h)
#define SIM_TIME_SCALE ((float)g_simScale)    // chon trong Setting (the Simulation: nut xN); mac dinh SIM_SCALE_DEFAULT

static float simTemp = SIM_AMBIENT;
static float simProbe = SIM_AMBIENT;      // nhiet do dau do (tre sau nhiet do trong long ~25 giay ao)

// Goi moi vong loop() (core 1). Cap nhat mo hinh nhiet moi 100 ms.
static void simUpdate() {
    static uint32_t last = 0;
    uint32_t now = millis();
    if (now - last < 100) return;
    last = now;

    const bool real = realSignalOk();
    bool justStarted = false;

    LOCK();
    if (g_simActive && real) {
        // Co tin hieu that: nhuong cho nhiet do that va tat gia lap
        g_simActive = false;
        bclock(); g_timeScale = 1.0f;
        g_simWanted = false;
    } else if (g_simWanted && !real) {
        if (!g_simActive) {
            g_simActive = true;
            bclock(); g_timeScale = SIM_TIME_SCALE;
            simTemp = simProbe = SIM_AMBIENT;
            justStarted = true;
        }
        // Mo hinh nhiet, tinh theo THOI GIAN AO (moi 0.1 s that = 0.1*SIM_TIME_SCALE giay ao):
        //   dT/dt = a*Heater - k*(T - Tmoi_truong),  k tang theo Air (va Cool)
        // Voi Heater 100%, Air 50%: tu 30 len 175 do C mat ~7 phut ao (~1 phut that khi SIM_TIME_SCALE = 7),
        // RoR toi da ~27 do C/phut (quy theo thoi gian ao) luc moi cap nhiet roi giam dan nhu may that.
        // Giu nhiet 175 do C can Heater khoang 45%. Bat Cool: nguoi nhanh (~4 phut ao ve gan 60 do C).
        // Charge (do hat vao): nhiet do hat tut manh nhu may that (TP) - chi de thu bieu do/TP
        static bool lastCharged = false;
        if (batchCharged && !lastCharged && simTemp > 120.0f) simTemp = SIM_AMBIENT + (simTemp - SIM_AMBIENT) * 0.45f;
        lastCharged = batchCharged;
        const float dt   = 0.1f * SIM_TIME_SCALE;                    // giay ao moi buoc
        const float heat = sendBuffer[HEAT_BYTE];                    // 0..100 %
        const float air  = sendBuffer[VENT_BYTE];                    // 0..100 %
        const bool  cool = sendBuffer[COOL_BYTE] > 0;
        const float heat_in = heat * 0.0045f;                        // do C / giay ao
        const float k = 0.0009f + 0.00001f * air + (cool ? 0.005f : 0.0f);   // he so mat nhiet (1/giay ao)
        simTemp += (heat_in - (simTemp - SIM_AMBIENT) * k) * dt;
        simTemp = constrain(simTemp, 0.0f, (float)300);
        if (g_timeScale != SIM_TIME_SCALE) { bclock(); g_timeScale = SIM_TIME_SCALE; }   // doi he so luc dang chay
        simProbe += (simTemp - simProbe) * fminf(1.0f, dt / 25.0f);    // dau do tre -> sau Charge BT tut dan roi tang lai (TP)
        temp = simProbe + random(-3, 4) * 0.02f;                     // nhieu do (khong cong don vao trang thai)
        lastTempMs = now;                                            // khong bi coi la mat tin hieu
    } else if (g_simActive) {
        g_simActive = false;                                         // nguoi dung tat gia lap
        bclock(); g_timeScale = 1.0f;
    }
    UNLOCK();

    if (justStarted) {                                               // dua may that ve trang thai tat
        static const uint8_t zero[CONTROLLER_LENGTH] = {0};
        sendRoasterFrame(zero);
    }
}

#else
static inline void simUpdate() {}
#endif
