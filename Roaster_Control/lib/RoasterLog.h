/*
 * GPL v3 - xem LyGia_Roaster.ino
 *
 * LOG ME RANG: khi bam END (me da Drop), neu du cac moc Charge, TP, DE, FC1s, Drop thi luu 1 file JSON vao phan vung FFat:
 *   /logs/0001.json, 0002.json ... (so thu tu tang dan, nho trong NVS, khong dung lai so cu). Thieu moc nao thi KHONG luu.
 *  - Dinh dang theo COT (de gon): bt10 / ror10 (x10, null = chua co RoR) / hp / fc, moi 1 giay tu luc Charge.
 *  - Chua co dong ho thuc (RTC) nen chua ghi ngay gio. Doc file ra may tinh / app: chua lam.
 *  - Bat/tat: ENABLE_ROAST_LOG (LySetting.h). Can ENABLE_ROAST_GRAPH = 1 va phan vung "16M Flash (3MB APP/9.9MB FATFS)".
 *  - Goi logInit() 1 lan trong setup(); goi logTick() moi vong loop() TRUOC graphUpdate() (core 1).
 */
#pragma once
#include "RoasterGraphData.h"
#include "RoasterProfile.h"

#if ENABLE_ROAST_LOG && ENABLE_ROAST_GRAPH
#include <FFat.h>
#include <Preferences.h>

#define LOG_DIR       "/logs"
#define LOG_MIN_FREE  200000                    // con it hon so byte nay thi khong luu (de danh cho he thong)

static bool     logFsOk = false;
static bool     logHad = false;                 // me dang mo o vong truoc
static uint32_t logEvMs[EV_COUNT];              // moc cua me dang mo (chup moi vong, vi END xoa cac moc)
static uint32_t logEvDone = 0;
static int      logCnt = 0, logTp = -1, logProf = -1;
static bool     logSim = false;
static char    *logPendBuf = nullptr;           // noi dung dang cho ghi (task ghi tu giai phong)
static size_t   logPendLen = 0;
static uint32_t logPendNo = 0;
static volatile bool logBusy = false;

static void logInit() {
    logFsOk = FFat.begin(true);                 // true: tu dinh dang neu phan vung chua co file he thong
    if (logFsOk && !FFat.exists(LOG_DIR)) FFat.mkdir(LOG_DIR);
}

static void logWriteJob(void *) {
    char path[32];
    snprintf(path, sizeof(path), LOG_DIR "/%04u.json", (unsigned)logPendNo);
    bool ok = false;
    if (logFsOk && (size_t)FFat.freeBytes() > (size_t)LOG_MIN_FREE + logPendLen) {
        File f = FFat.open(path, "w");
        if (f) { ok = (f.write((const uint8_t *)logPendBuf, logPendLen) == logPendLen); f.close(); }
    }
    if (ok) { Preferences p; p.begin("lylog", false); p.putUInt("n", logPendNo); p.end(); }   // chi tang so khi da ghi xong
    free(logPendBuf); logPendBuf = nullptr; logPendLen = 0;
    logBusy = false;
#ifndef LOG_SYNC_WRITE
    vTaskDelete(nullptr);
#endif
}

// Du cac moc chua: Charge, TP, DE, FC1s, Drop
static bool logComplete() {
    const uint32_t need = (1u << EV_CHARGE) | (1u << EV_DE) | (1u << EV_FC1) | (1u << EV_DROP);
    return (logEvDone & need) == need && logTp >= 0 && logCnt > 10
           && logEvMs[EV_DE] != EV_NO_TIME && logEvMs[EV_FC1] != EV_NO_TIME && logEvMs[EV_DROP] != EV_NO_TIME;
}

static void logBuildAndSave() {
    if (!logFsOk || logBusy) return;
    uint32_t n = 0;
    { Preferences p; p.begin("lylog", true); n = p.getUInt("n", 0); p.end(); }
    n++;
    const size_t cap = 4096 + (size_t)logCnt * 22;                  // moi giay: bt10 6 + ror10 6 + hp 4 + fc 4 + dau phay
    char *b = (char *)heap_caps_malloc(cap, MALLOC_CAP_SPIRAM);
    if (!b) b = (char *)malloc(cap);
    if (!b) return;
    size_t o = 0;
    o += snprintf(b + o, cap - o, "{\"format\":\"lygia-roastlog\",\"version\":1,\"fw\":\"%s\",\"name\":\"#%04u\",", FW_VERSION, (unsigned)n);
#if ENABLE_PROFILE
    if (logProf >= 0 && logProf < PROFILE_COUNT) o += snprintf(b + o, cap - o, "\"profile\":\"%s\",", PROFILE_FACTORY[logProf].name);
    else o += snprintf(b + o, cap - o, "\"profile\":null,");
#else
    o += snprintf(b + o, cap - o, "\"profile\":null,");
#endif
    const int tEv[4] = { 0, logTp, (int)(logEvMs[EV_DE] / 1000), (int)(logEvMs[EV_FC1] / 1000) };
    const int tDrop = (int)(logEvMs[EV_DROP] / 1000);
    auto bt10 = [&](int t) { return (t >= 0 && t < logCnt) ? (int)gBT[t] : (int)gBT[logCnt - 1]; };
    const int dry = tEv[2], mai = tEv[3] - tEv[2], dev = tDrop - tEv[3];
    const float dtr = tDrop > 0 ? 100.0f * (float)dev / (float)tDrop : 0.0f;
    o += snprintf(b + o, cap - o,
        "\"unit\":\"C\",\"sim\":%s,\"duration_s\":%d,"
        "\"events\":{\"C\":{\"t\":0,\"bt10\":%d},\"TP\":{\"t\":%d,\"bt10\":%d},\"DE\":{\"t\":%d,\"bt10\":%d},\"FC1s\":{\"t\":%d,\"bt10\":%d},\"D\":{\"t\":%d,\"bt10\":%d}},"
        "\"phases\":{\"dry_s\":%d,\"maillard_s\":%d,\"dev_s\":%d,\"dtr_pct\":%.1f},\"step_s\":1,",
        logSim ? "true" : "false", tDrop,
        bt10(0), tEv[1], bt10(tEv[1]), tEv[2], bt10(tEv[2]), tEv[3], bt10(tEv[3]), tDrop, bt10(tDrop),
        dry, mai, dev, dtr);
    const int N = logCnt < (int)GRAPH_MAX_S ? logCnt : (int)GRAPH_MAX_S;
    o += snprintf(b + o, cap - o, "\"bt10\":[");
    for (int i = 0; i < N; i++) o += snprintf(b + o, cap - o, i ? ",%d" : "%d", (int)gBT[i]);
    o += snprintf(b + o, cap - o, "],\"ror10\":[");
    for (int i = 0; i < N; i++) { if (gROR[i] == GRAPH_NO_ROR) o += snprintf(b + o, cap - o, i ? ",null" : "null"); else o += snprintf(b + o, cap - o, i ? ",%d" : "%d", (int)gROR[i]); }
    o += snprintf(b + o, cap - o, "],\"hp\":[");
    for (int i = 0; i < N; i++) o += snprintf(b + o, cap - o, i ? ",%d" : "%d", (int)gHP[i]);
    o += snprintf(b + o, cap - o, "],\"fc\":[");
    for (int i = 0; i < N; i++) o += snprintf(b + o, cap - o, i ? ",%d" : "%d", (int)gFC[i]);
    o += snprintf(b + o, cap - o, "]}");
    if (o >= cap) { free(b); return; }
    logPendBuf = b; logPendLen = o; logPendNo = n; logBusy = true;
#ifdef LOG_SYNC_WRITE
    logWriteJob(nullptr);
#else
    if (xTaskCreatePinnedToCore(logWriteJob, "lylog", 4096, nullptr, 1, nullptr, 0) != pdPASS) logWriteJob(nullptr);   // khong tao duoc task: ghi truc tiep
#endif
}

static void logTick() {
    LOCK();
    if (batchLive) {                                                // chup trang thai me dang mo (END se xoa cac moc va du lieu)
        for (int i = 0; i < EV_COUNT; i++) logEvMs[i] = evMs[i];
        logEvDone = evDone; logCnt = gCount; logTp = gTpIdx; logSim = g_simActive;
#if ENABLE_PROFILE
        logProf = g_prof_sel;
#endif
        logHad = true;
    } else if (logHad) {                                            // vua END
        logHad = false;
        if (logComplete()) logBuildAndSave();
    }
    UNLOCK();
}

#else
static inline void logInit() {}
static inline void logTick() {}
#endif
