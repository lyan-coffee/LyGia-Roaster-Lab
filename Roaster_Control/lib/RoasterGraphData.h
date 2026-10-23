/*
 * GPL v3 - xem LyGia_Roaster.ino
 *
 * MODULE BIEU DO RANG - PHAN DU LIEU (khong phu thuoc LVGL):
 *  - Lay mau nhiet do hat (BT) va RoR moi giay, tinh tu luc Charge (cung moc voi dong ho Time), toi da GRAPH_MAX_S giay.
 *  - Du lieu duoc xoa khi bam END (ket thuc me). Sau Drop du lieu van giu de xem.
 *  - Phan giao dien: lib/RoasterGraph.h.
 *  - Chuong trinh chinh chi can: #include file nay + goi graphUpdate() trong loop() (xem LyGia_Roaster.ino).
 *  - ENABLE_ROAST_GRAPH 0: bo han chuc nang (graphUpdate() la ham rong, khong ton RAM).
 */
#pragma once
#include "SkiCMD.h"


#if ENABLE_ROAST_GRAPH

#define GRAPH_MAX_S      1200         // so giay toi da luu (1200 = 20 phut)
#define GRAPH_NO_ROR     ((int16_t)-32768)

static int16_t  gBT[GRAPH_MAX_S];     // nhiet do hat x10 (do C)
static int16_t  gROR[GRAPH_MAX_S];    // RoR x10 (do C/phut) hoac GRAPH_NO_ROR
#if ENABLE_ROAST_LOG
static uint8_t  gHP[GRAPH_MAX_S];     // Heater % moi giay (chi de ghi log)
static uint8_t  gFC[GRAPH_MAX_S];     // Air % moi giay (chi de ghi log)
#endif
static volatile uint16_t gCount = 0;  // so mau hop le (chi so = giay ke tu Charge)
static volatile int gTpIdx = -1;      // TP (turning point): giay ke tu Charge cua diem BT thap nhat, -1 = chua xac nhan (BT chua tang >= 1 do tu diem thap nhat)

static void graphReset() { gCount = 0; gTpIdx = -1; }

// Goi moi vong loop() (core 1). Tu khoa bang LOCK()/UNLOCK().
static void graphUpdate() {
    LOCK();
    if (!batchCharged) {
        if (gCount != 0) graphReset();
    } else if (batchRunning) {
        const int idx = (int)(batchElapsedMs() / 1000);
        while ((int)gCount <= idx && gCount < GRAPH_MAX_S) {
            const int i = gCount;
            const float t = constrain((float)temp, -50.0f, 300.0f);
            gBT[i]  = (int16_t)lroundf(t * 10.0f);
            gROR[i] = rorOk ? (int16_t)lroundf(constrain(rorValue, -300.0f, 300.0f) * 10.0f) : GRAPH_NO_ROR;
#if ENABLE_ROAST_LOG
            gHP[i] = sendBuffer[HEAT_BYTE]; gFC[i] = sendBuffer[VENT_BYTE];
#endif
            gCount = i + 1;
        }
        if (gTpIdx < 0 && gCount > 1) {                       // TP: dung lai khi da xac nhan, khong doi nua
            int mi = 0;
            for (int i = 1; i < (int)gCount; i++) if (gBT[i] < gBT[mi]) mi = i;
            if (gBT[gCount - 1] >= gBT[mi] + 10) gTpIdx = mi;
        }
    }
    UNLOCK();
}

#else
static inline void graphUpdate() {}
#endif
