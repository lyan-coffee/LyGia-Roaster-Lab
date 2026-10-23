/*
 * GPL v3 - xem LyGia_Roaster.ino
 */
#pragma once
#include <PID_v1.h>
#include "SkiSync.h"
#include "SkiPIDConfig.h"

// =============================================================================
// LOI DIEU KHIEN MAY RANG
//  - sendBuffer la "nguon su that" duy nhat cua Heater/Air/Drum/Cool.
//  - Man hinh cam ung (panel*) va HiBean (parseAndExecuteCommands) deu sua qua
//    cac ham o day -> hai ben luon dong bo.
//  - Cac ham chi sua bo dem; viec phat khung xuong may rang do loop() lam.
// =============================================================================

#define DEFAULT_PID_SETPOINT 175     // muc nhiet Set mac dinh khi dung PID (do C); nut -/+ doi PID_STEP_C (RoasterUI.h)

extern double temp;          // do C, cap nhat tu loop()
extern char CorF;            // don vi HiBean yeu cau
extern PID myPID;
extern PIDConfig myPIDConfig;
extern double pInput, pOutput, pSetpoint;   // pSetpoint: do C
extern int manualHeatLevel;

// Timing giao thuc may rang
const int PULSE_ZERO       = 650;
const int POST_PULSE_DELAY = 750;
const int START_PULSE      = 7500;
const int START_DELAY      = 3800;

const int CONTROLLER_LENGTH = 6;
uint8_t sendBuffer[CONTROLLER_LENGTH];

enum ControlBytes {
    VENT_BYTE = 0, FILTER_BYTE = 1, COOL_BYTE = 2, DRUM_BYTE = 3, HEAT_BYTE = 4, CHECK_BYTE = 5
};

enum Src : uint8_t { SRC_PANEL = 0, SRC_HIBEAN = 1 };

// Trang thai phu (bao ve bang LOCK)
static Src      heatSrc       = SRC_PANEL;   // ai dang dieu khien Heater
static uint32_t lastTempMs    = 0;           // lan cuoi nhan nhiet do hop le
static bool     roasting      = false;
static uint32_t roastStartMs  = 0;
static uint32_t roastFrozenMs = 0;
static uint32_t lastRealTempMs = 0;          // lan cuoi nhan nhiet do THAT (khong tinh gia lap)
static volatile bool g_simWanted = (DEFAULT_SIMULATION);   // nguoi dung muon gia lap
static volatile bool g_simActive = false;                  // gia lap dang chay (chi khi khong co tin hieu that)
static volatile float g_timeScale = 1.0f;                  // he so thoi gian cua nguon nhiet do (1 = that; module gia lap dat > 1). RoR chia cho he so nay
// DONG HO ME RANG (Time, bieu do, moc, RoR): chay theo "thoi gian cua nguon nhiet do". Binh thuong = millis().
// Khi gia lap (g_timeScale = 7) dong ho chay nhanh gap 7 lan, cung nhip voi nhiet do gia lap -> Time, bieu do, moc khop nhau.
// Cac moc cho xac nhan (Drop / END 5 giay, goi y) van dung millis() that. Goi truoc moi lan doi g_timeScale.
static uint32_t bclock() {
    static uint32_t last = 0, v = 0;
    LOCK();
    const uint32_t m = millis();
    v += (uint32_t)((float)(m - last) * g_timeScale + 0.5f);
    last = m;
    const uint32_t r = v;
    UNLOCK();
    return r;
}
static char     hintBuf[24]   = "";
static uint32_t hintUntil     = 0;

// Co tin hieu nhiet do that tu may rang trong 3 giay gan day?
static bool realSignalOk() { return lastRealTempMs != 0 && (millis() - lastRealTempMs) < 3000; }

// ---- prototypes ----
static void cutHeat(const char *why);
#define AIR_LOW_MSG "Need AIR >= " CFG_STR(MIN_AIR_FOR_HEAT) "%"   // so % lay theo MIN_AIR_FOR_HEAT (LySetting.h)
static void enforceInterlock();
void handleHEAT(int value);

// -----------------------------------------------------------------------------
// Tien ich
// -----------------------------------------------------------------------------
static void setHint(const char *t) {
    LOCK();
    strncpy(hintBuf, t, sizeof(hintBuf) - 1);
    hintBuf[sizeof(hintBuf) - 1] = 0;
    hintUntil = millis() + 2500;
    UNLOCK();
}

static void setControlChecksum() {
    uint8_t sum = 0;
    for (int i = 0; i < (CONTROLLER_LENGTH - 1); i++) sum += sendBuffer[i];
    sendBuffer[CHECK_BYTE] = sum;
}

static void setValue(uint8_t *bytePtr, uint8_t value) {
    LOCK();
    *bytePtr = value;
    setControlChecksum();
    UNLOCK();
}

// ---- Cac moc me rang (PreHeat, Charge, DE = Dry End, 1FC, 2FC, Drop) + dong ho "Time" (tinh tu luc Charge) ----
enum { EV_PREHEAT = 0, EV_CHARGE, EV_DE, EV_FC1, EV_FC2, EV_DROP, EV_COUNT };
#define EV_NO_TIME 0xFFFFFFFFu             // moc khong co gio (vi du Drop khi dong ho Time chua chay)
static uint8_t  evDone        = 0;           // bit i = moc i da bam
static uint32_t evMs[EV_COUNT] = {0};        // thoi diem moc (ms tinh tu Charge)
static float    evTemp[EV_COUNT] = {0};      // nhiet do BT (do C) luc bam moc
static bool     batchCharged  = false;       // da bam Charge
static bool     batchRunning  = false;       // dong ho Time dang chay
static uint32_t chargeMs      = 0;
static uint32_t batchFrozenMs = 0;
static uint32_t dropArmUntil  = 0;           // != 0: dang cho xac nhan Drop den moc millis() nay
#if ENABLE_PROFILE
static volatile int g_prof_sel = -1;       // Profile dang chon cho me rang nay (-1 = Manual / khong co)
#endif
// Auto dang chay theo Profile: cac luat cua Profile thay the preset PR / CH
#if ENABLE_PROFILE && ENABLE_PROFILE_AUTO
#define PROF_AUTO_ACTIVE() (g_prof_sel >= 0)
#else
#define PROF_AUTO_ACTIVE() (false)
#endif
static bool     batchLive     = false;       // me rang dang "mo": tu START den END (ke ca sau Drop, khi dang lam nguoi / xem lai bieu do)
static uint32_t endArmUntil   = 0;           // != 0: dang cho xac nhan END (chi khi chua Drop) den moc millis() nay

static uint32_t batchElapsedMs() { return batchRunning ? (bclock() - chargeMs) : batchFrozenMs; }
static void batchFreeze() { if (batchRunning) { batchFrozenMs = bclock() - chargeMs; batchRunning = false; } dropArmUntil = 0; }
static void batchReset()  { evDone = 0; for (int i = 0; i < EV_COUNT; i++) { evMs[i] = 0; evTemp[i] = 0; }
                            batchCharged = batchRunning = false; chargeMs = batchFrozenMs = 0; dropArmUntil = 0; endArmUntil = 0; }

// ---- RoR (toc do tang nhiet, do C/phut): lay mau BT moi giay, so voi 30 giay truoc. chia cho g_timeScale (=1 neu khong gia lap) ----
#define ROR_WINDOW_S 30
#define ROR_MIN_SHOW 1.0f                  // chi hien so RoR khi >= muc nay (do C/phut); gia tri sat 0 hay nhap nhay "--"
#define ROR_HYST     0.2f                  // da hien thi thi chi an khi tut xuong duoi (ROR_MIN_SHOW - ROR_HYST): quanh nguong khong chop
static float    rorBuf[ROR_WINDOW_S + 1];
static int      rorHead = 0, rorCnt = 0;
static float    rorValue = 0.0f;
static bool     rorOk = false;
static uint32_t rorLastMs = 0;

static uint32_t roastElapsedMs() { return roasting ? (bclock() - roastStartMs) : roastFrozenMs; }
static void roastBegin() { roasting = true; batchLive = true; roastStartMs = bclock(); roastFrozenMs = 0; }
static void roastEnd()   { batchFreeze(); if (roasting) { roastFrozenMs = bclock() - roastStartMs; roasting = false; } }

// Ly do chan Heater (nullptr = duoc phep). Nut START chi bat buoc voi lenh tu man hinh.
static const char *needStartHint() { return batchLive ? "Press END" : "Press START"; }   // da START nhung roasting = false (da Drop / Cool bat): can END
static const char *heatBlockReason(Src src) {
    if (evDone & (1 << EV_DROP)) return "Press END";                 // sau Drop: Heater khoa cho den khi END (HiBean cung vay)
    if (src == SRC_PANEL && !roasting) return needStartHint();
#if SAFETY_INTERLOCK
    if (sendBuffer[DRUM_BYTE] == 0)               return "Need DRUM ON";
    if (sendBuffer[COOL_BYTE] > 0)                return "Cool fan is ON";
    if (sendBuffer[VENT_BYTE] < MIN_AIR_FOR_HEAT) return AIR_LOW_MSG;
#endif
    return nullptr;
}

// Tat PID nhung GIU NGUYEN muc Heater hien tai (chuyen sang thu cong, khong ve 0)
static void pidOffKeepHeat() {
    LOCK();
    if (myPID.GetMode() == AUTOMATIC) {
        myPID.SetMode(MANUAL);
        manualHeatLevel = sendBuffer[HEAT_BYTE];
    }
    UNLOCK();
}

// Cat Heater + thoat PID
static void cutHeat(const char *why) {
    LOCK();
    if (myPID.GetMode() == AUTOMATIC) myPID.SetMode(MANUAL);
    manualHeatLevel = 0;
    setValue(&sendBuffer[HEAT_BYTE], 0);
    if (why) setHint(why);
    UNLOCK();
}

static void enforceInterlock() {
    LOCK();
    bool heating = sendBuffer[HEAT_BYTE] > 0 || myPID.GetMode() == AUTOMATIC;
    if (heating) {
        // Dang gia nhiet ma Air thap / Drum OFF / Cool ON: chi CANH BAO, KHONG tu cat Heater.
        // Heater chi bi cat khi Drop hoac END (hoac dung khan cap / mat tin hieu o watchdog).
        const char *r = heatBlockReason(heatSrc);
        if (r) setHint(r);
    }
    UNLOCK();
}

// Watchdog: mat HiBean / mat tin hieu nhiet do khi dang cap nhiet -> KHONG cat Heater (cat la hu me rang),
// chi canh bao; neu dang PID thi chuyen sang thu cong va GIU nguyen muc Heater de nguoi rang tu xu ly.
static void safetyWatchdog() {
    LOCK();
    if (sendBuffer[HEAT_BYTE] > 0 || myPID.GetMode() == AUTOMATIC) {
        { const char *r = heatBlockReason(heatSrc); if (r) setHint(r); }   // Air thap / Drum OFF / Cool ON: nhac lien tuc, khong cat Heater
        uint32_t now = millis();
        const char *lost = nullptr;
        if (heatSrc == SRC_HIBEAN && (now - g_lastHiBeanMs) > HIBEAN_TIMEOUT_MS) lost = "HiBean lost";
        else if (TEMP_STALE_MS > 0 && (now - lastTempMs) > TEMP_STALE_MS)         lost = "No temp signal";
        if (lost) {
            setHint(lost);
            if (myPID.GetMode() == AUTOMATIC) pidOffKeepHeat();   // PID khong con y nghia khi mat tin hieu: rang tay, giu muc Heater
        }
    }
    UNLOCK();
}

// -----------------------------------------------------------------------------
// Bo xu ly lenh (dung chung cho HiBean va man hinh)
// -----------------------------------------------------------------------------
void handleHEAT(int value) {
    if (value >= 0 && value <= 100) setValue(&sendBuffer[HEAT_BYTE], (uint8_t)value);
}

// Loc (filter) suy ra tu Vent: 1-25 -> 4, 26-50 -> 3, 51-75 -> 2, 76-100 -> 1
static void syncFilterFromVent() {
    int v = sendBuffer[VENT_BYTE];
    int f = (v == 0) ? 0 : (4 - ((v - 1) * 4 / 100));
    setValue(&sendBuffer[FILTER_BYTE], (uint8_t)f);
}

void handleOT1(int value, Src src = SRC_HIBEAN) {
    LOCK();
    if (myPID.GetMode() == MANUAL) {
        int v = constrain(value, 0, 100);
        if (v > 0) {
            const char *r = heatBlockReason(src);
            if (r) { setHint(r); v = 0; }
            else if (src == SRC_HIBEAN && HIBEAN_AUTOSTART && !roasting) roastBegin();
        }
        manualHeatLevel = v;
        heatSrc = src;
        handleHEAT(v);
    } else {
        // PID dang chay: OT1 = cong suat toi da
        myPIDConfig.setMaxPower(constrain(value, 0, 100));
        myPIDConfig.apply(myPID);
    }
    UNLOCK();
}

void handleVENT(int value) {
    if (value < 0 || value > 100) return;
    LOCK();
    setValue(&sendBuffer[VENT_BYTE], (uint8_t)value);
    syncFilterFromVent();
    enforceInterlock();
    UNLOCK();
}

void handleFILTER(int value) {
    if (value >= 0 && value <= 4) setValue(&sendBuffer[FILTER_BYTE], (uint8_t)value);
}

void handleDRUM(int value) {
    LOCK();
    setValue(&sendBuffer[DRUM_BYTE], value != 0 ? 100 : 0);
    enforceInterlock();
    UNLOCK();
}

void handleCOOL(int value) {
    if (value < 0 || value > 100) return;
    LOCK();
    setValue(&sendBuffer[COOL_BYTE], (uint8_t)value);
    // Cool chi la quat giai nhiet: KHONG ket thuc me, khong dung dong ho, khong doi gi khac.
    // (Interlock an toan van tu cat Heater khi Cool ON - Heater khong duoc chay cung Cool.)
    if (value > 0) enforceInterlock();
    syncFilterFromVent();                            // giu filter theo Vent
    UNLOCK();
}

void eStop() {
    LOCK();
    cutHeat("E-STOP");
    handleVENT(100);
    UNLOCK();
}

void roasterShutdown() {
    LOCK();
    for (int i = 0; i < CONTROLLER_LENGTH; i++) sendBuffer[i] = 0;
    if (myPID.GetMode() == AUTOMATIC) myPID.SetMode(MANUAL);
    manualHeatLevel = 0;
    roastEnd();
    UNLOCK();
}

void setPIDMode(bool usePID) {
    LOCK();
    if (usePID) {
        heatSrc = SRC_HIBEAN;
        const char *r = heatBlockReason(SRC_HIBEAN);
        if (r) {
            setHint(r);
        } else {
            if (HIBEAN_AUTOSTART && !roasting) roastBegin();
            pInput  = temp;                          // khoi dau "em" (bumpless): PID bat dau tu cong suat Heater hien tai,
            pOutput = sendBuffer[HEAT_BYTE];         // khong dung gia tri PID cu con luu tu lan truoc
            myPID.SetMode(AUTOMATIC);
        }
    } else {
        pidOffKeepHeat();                  // tat PID: giu nguyen muc Heater hien tai
    }
    UNLOCK();
}

// PID: chay trong loop() (core 1)
void handlePIDControl() {
    LOCK();
    if (myPID.GetMode() == AUTOMATIC) {
        pInput = temp;
        if (myPIDConfig.getPMode() == P_ON_E && pSetpoint - temp > PID_I_BAND_C) {
            // Con xa Set: khong de tich phan "no" len 100% (se vot qua Set). Nap lai tich phan = PID_I_PRESET, Heater chay theo Kp*sai so
            pOutput = PID_I_PRESET;
            myPID.SetMode(MANUAL);
            myPID.SetMode(AUTOMATIC);        // Initialize(): outputSum = pOutput, lastInput = pInput
        }
        myPID.Compute();
        double hd = pOutput;
#if ENABLE_PID_ASSIST
        {   // Ho tro tang nhiet (ap cho moi bo PID): con xa Set thi ep Heater cao toi thieu, vuot Set thi bop/cat Heater
            const double lead = (rorOk ? (double)rorValue : 0.0) * PID_LEAD_S / 60.0;   // BT du kien sau PID_LEAD_S giay (RoR do C/phut)
            const double gap  = pSetpoint - (temp + (lead > 0 ? lead : 0.0));             // khoang cach toi Set (da tru phan dang tang)
            static bool boost = false;                                                     // "dang leo": bat khi cach Set > FAR, tat khi sat Set (khong bat lai vi dao dong nho luc giu nhiet)
            if (gap > PID_BOOST_FAR_C) boost = true; else if (gap <= PID_BOOST_END_C) boost = false;
            if (boost && gap > PID_BOOST_FAR_C)       { if (hd < PID_BOOST_FAR_PCT)  hd = PID_BOOST_FAR_PCT; }
            else if (boost && gap > PID_BOOST_NEAR_C) { if (hd < PID_BOOST_NEAR_PCT) hd = PID_BOOST_NEAR_PCT; }
            const double over = temp - pSetpoint;                                          // do C vuot Set
            if (over > PID_OVER_MAX_C)       hd = 0;
            else if (over > PID_OVER_SOFT_C) { if (hd > PID_OVER_SOFT_PCT) hd = PID_OVER_SOFT_PCT; }
        }
#endif
        int h = (int)(round(hd / 5.0) * 5);
        handleHEAT(constrain(h, 0, 100));
    }
    UNLOCK();
}

// -----------------------------------------------------------------------------
// Hanh dong tu man hinh cam ung
// -----------------------------------------------------------------------------
void panelSetHeater(int v) {
    v = constrain(v, 0, 100);
    v = ((v + 2) / 5) * 5;
    LOCK();
    if (myPID.GetMode() == AUTOMATIC) pidOffKeepHeat();     // cham tay = chuyen sang thu cong, Heater lay theo gia tri vua cham (khong ve 0)
    handleOT1(v, SRC_PANEL);
    UNLOCK();
}
// Bat/tat PID tu man hinh (chi bat duoc khi du dieu kien an toan nhu Heater)
void panelSetPID(bool on) {
    LOCK();
    if (on) {
        const char *r = heatBlockReason(SRC_PANEL);
        if (r) {
            setHint(r);
        } else {
            if (pSetpoint <= 0) pSetpoint = DEFAULT_PID_SETPOINT;
            heatSrc = SRC_PANEL;
            pInput  = temp;                          // khoi dau "em" (bumpless)
            pOutput = sendBuffer[HEAT_BYTE];
            myPID.SetMode(AUTOMATIC);
        }
    } else {
        pidOffKeepHeat();                 // tat PID: giu nguyen muc Heater hien tai, sang thu cong
    }
    UNLOCK();
}
// Muc nhiet Set (SV), do C
void panelSetSetpoint(float c) {
    LOCK();
    pSetpoint = constrain((double)(round(c / 5.0f) * 5), 0.0, 300.0);   // bo 5 do C
    UNLOCK();
}
// Cat Heater ngay (tat luon PID) - dung cho nut "Heater Off" tren man hinh canh bao qua nhiet
void panelHeaterOff() {
    LOCK();
    cutHeat(nullptr);
    UNLOCK();
}
void panelSetAir(int v)     { handleVENT(constrain(v, 0, 100)); }
void panelSetDrum(bool on)  { handleDRUM(on ? 100 : 0); }
void panelSetCool(bool on)  { handleCOOL(on ? 100 : 0); }

void panelStart() {
    LOCK();
    if (batchLive) { setHint("Press END"); UNLOCK(); return; }   // mot me dang mo: phai END truoc
    batchReset();                      // START = bat dau me moi: xoa cac moc va dong ho Time
    handleCOOL(0);
    setValue(&sendBuffer[DRUM_BYTE], 100);
    if (sendBuffer[VENT_BYTE] < MIN_AIR_FOR_HEAT) handleVENT(MIN_AIR_FOR_HEAT);
    roastBegin();
    UNLOCK();
}

// END: ket thuc me rang va xoa sach (bieu do, cac moc, dong ho). Chua Drop: bam lan 1 -> cho xac nhan 5 giay, lan 2 -> xong.
// Da Drop: ket thuc ngay, khong hoi. Cool / Drum / Air giu nguyen (hat co the dang nguoi).
void panelEnd() {
    LOCK();
    const uint32_t now = millis();
    if (!batchLive && !roasting) { UNLOCK(); return; }
    const bool dropped = (evDone & (1 << EV_DROP)) != 0;
    if (!dropped && !(endArmUntil != 0 && (int32_t)(endArmUntil - now) > 0)) {
        endArmUntil = now + END_CONFIRM_MS;
        UNLOCK();
        return;
    }
    roastEnd();
    cutHeat(nullptr);
    batchReset();
    batchLive = false;
    g_prof_sel = -1;                   // END: bo profile da chon -> duong mau + ten profile tren Graph mat (log da chup ten o lan logTick truoc)
    UNLOCK();
}

// Goi moi vong loop() (core 1); lay mau moi 1 GIAY theo dong ho bclock() (= thoi gian cua nguon nhiet do).
// Khi gia lap chay x7 thi 1 giay ao = ~143 ms that: cua so 30 giay la 30 giay AO, RoR tinh theo phut AO (cung nhip voi Time / bieu do / nhiet do gia lap).
// Khong gia lap (g_timeScale = 1) thi giong het truoc day.
static void rorUpdate() {
    const uint32_t vnow = bclock();                          // dong ho ao: dung de chon nhip lay mau
    if (vnow - rorLastMs < 1000) return;
    rorLastMs = vnow;
    const uint32_t now = millis();                           // moc that: kiem tra mat tin hieu nhiet
    LOCK();
    static bool lastSim = false;
    if (g_simActive != lastSim) { lastSim = g_simActive; rorCnt = 0; rorHead = 0; rorOk = false; }   // doi nguon nhiet do -> tinh lai
    const bool ok = (lastTempMs != 0) && ((now - lastTempMs) < TEMP_SIGNAL_OK_MS);
    if (!ok) {
        rorCnt = 0; rorHead = 0; rorOk = false;               // mat tin hieu -> tinh lai tu dau
    } else {
        rorBuf[rorHead] = (float)temp;
        rorHead = (rorHead + 1) % (ROR_WINDOW_S + 1);
        if (rorCnt < ROR_WINDOW_S + 1) rorCnt++;
        if (rorCnt == ROR_WINDOW_S + 1) {                     // du 30 giay: rorHead tro vao mau cu nhat
            rorValue = ((float)temp - rorBuf[rorHead]) * 60.0f / (float)ROR_WINDOW_S;   // do C / phut (phut ao khi gia lap)
            rorOk = true;
        } else {
            rorOk = false;
        }
    }
    UNLOCK();
}

// Bam cac moc PreHeat / Charge / DE / 1FC / 2FC (Drop co ham rieng: panelDrop)
static bool presetUnitIsF();                       // dinh nghia o RoasterSettings.h (SkiCMD khong thay g_set)
void panelEvent(int ev) {
    if (ev < 0 || ev >= EV_COUNT || ev == EV_DROP) return;
    LOCK();
    if (evDone & (1 << ev)) { UNLOCK(); return; }             // moc da bam
    if (!roasting) { setHint(needStartHint()); UNLOCK(); return; }   // chi bam duoc khi dang rang (da START)
    if (ev == EV_PREHEAT) {
        if (batchCharged) { UNLOCK(); return; }               // PreHeat chi truoc Charge
        evDone |= (1 << ev);
#if ENABLE_EVENT_PRESETS
        // PR: Air 10%, bat PID giu PRESET_PR_TEMP_C (hoac PRESET_PR_TEMP_F neu dang hien do F). Auto theo Profile: luat Preheat cua Profile lam viec nay
        if (!PROF_AUTO_ACTIVE()) {
            handleVENT(PRESET_PR_AIR);
            pSetpoint = presetUnitIsF() ? (PRESET_PR_TEMP_F - 32) / 1.8 : (double)PRESET_PR_TEMP_C;
            panelSetPID(true);
        }
#endif
    } else if (ev == EV_CHARGE) {
        batchCharged = true; batchRunning = true;
        chargeMs = bclock(); batchFrozenMs = 0; evMs[EV_CHARGE] = 0; evTemp[EV_CHARGE] = (float)temp;
        evDone |= (1 << ev);
#if ENABLE_EVENT_PRESETS
        // CH: tat PID, Air 20%, Heater 70% (dat Air truoc de khong vuong canh bao Air thap). Auto theo Profile: luat Charge cua Profile
        if (!PROF_AUTO_ACTIVE()) {
            pidOffKeepHeat();
            handleVENT(PRESET_CH_AIR);
            panelSetHeater(PRESET_CH_HEATER);
        }
#endif
    } else {
        if (!batchRunning) { setHint("Press Charge"); UNLOCK(); return; }
        evMs[ev] = bclock() - chargeMs; evTemp[ev] = (float)temp;
        evDone |= (1 << ev);
    }
    UNLOCK();
}

// Huy cac xac nhan Drop / END dang cho (nguoi dung bam Huy tren hop thoai)
void panelCancelConfirm() { LOCK(); dropArmUntil = 0; endArmUntil = 0; UNLOCK(); }

// Drop: chi bam duoc khi da START, nhung KHONG can den 2FC (tham chi chua Charge van duoc).
// Bam lan 1 = cho xac nhan (tu huy sau 5 giay); bam lan 2 = xa hat:
// bat Cool, tat Heater (thoat PID), dung dong ho Time.
// Neu dong ho Time chua chay (chua Charge) thi Drop khong ghi gio (EV_NO_TIME) - giao dien khong hien so gio.
void panelDrop() {
    LOCK();
    const uint32_t now = millis();
    if (evDone & (1 << EV_DROP)) {
        // da Drop roi: cho START moi
    } else if (!roasting) {
        setHint(needStartHint());
    } else if (dropArmUntil != 0 && (int32_t)(dropArmUntil - now) > 0) {
        dropArmUntil = 0;
        evMs[EV_DROP] = batchRunning ? (bclock() - chargeMs) : EV_NO_TIME; evTemp[EV_DROP] = (float)temp;
        evDone |= (1 << EV_DROP);
        roastEnd();                                           // Drop: dung dong ho + cat Heater, bat Cool, Air toi da
        cutHeat(nullptr);
        handleCOOL(100);
        handleVENT(100);
    } else {
        dropArmUntil = now + DROP_CONFIRM_MS;
    }
    UNLOCK();
}

// -----------------------------------------------------------------------------
// Anh chup trang thai cho giao dien
// -----------------------------------------------------------------------------
struct Snap {
    int   heat, vent, drum, cool;
    bool  roasting, pid, tempOk, sim;
    float temp, sp;
    uint32_t elapsed;
    char  hint[24];
    // Time / RoR / moc me rang
    uint32_t batchMs;           // dong ho Time (ms)
    bool     batchRun, charged;
    bool     rorOk;
    float    ror;               // do C / phut
    uint8_t  evDone;
    uint32_t evMs[EV_COUNT];
    float    evTemp[EV_COUNT];   // BT (do C) luc bam moc
    uint32_t dropArmMs;         // > 0: con bao nhieu ms de xac nhan Drop
    bool     live;              // me rang dang mo (START ... END)
    uint32_t endArmMs;          // > 0: con bao nhieu ms de xac nhan END
};

static void roasterSnapshot(Snap &s) {
    LOCK();
    s.heat = sendBuffer[HEAT_BYTE];
    s.vent = sendBuffer[VENT_BYTE];
    s.drum = sendBuffer[DRUM_BYTE];
    s.cool = sendBuffer[COOL_BYTE];
    s.roasting = roasting;
    s.live = batchLive;
    s.sim = g_simActive;
    s.pid = (myPID.GetMode() == AUTOMATIC);
    s.temp = (float)temp;
    s.sp = (float)pSetpoint;
    s.tempOk = (lastTempMs != 0) && ((millis() - lastTempMs) < TEMP_SIGNAL_OK_MS);
    s.elapsed = roastElapsedMs();
    s.batchMs = batchElapsedMs();
    s.batchRun = batchRunning;
    s.charged = batchCharged;
    s.rorOk = rorOk;
    s.ror = rorValue;
    s.evDone = evDone;
    for (int i = 0; i < EV_COUNT; i++) { s.evMs[i] = evMs[i]; s.evTemp[i] = evTemp[i]; }
    {
        uint32_t nowMs = millis();
        s.dropArmMs = (dropArmUntil != 0 && (int32_t)(dropArmUntil - nowMs) > 0) ? (dropArmUntil - nowMs) : 0;
        s.endArmMs  = (endArmUntil != 0 && (int32_t)(endArmUntil - nowMs) > 0) ? (endArmUntil - nowMs) : 0;
    }
    if (millis() < hintUntil) strncpy(s.hint, hintBuf, sizeof(s.hint)); else s.hint[0] = 0;
    s.hint[sizeof(s.hint) - 1] = 0;
    UNLOCK();
}

// -----------------------------------------------------------------------------
// Tra loi READ cho HiBean: "0,BT,BT,heat,vent"  (heat/vent la gia tri THUC cua may,
// nen thay doi tren man hinh se hien len HiBean o lan READ ke tiep)
// -----------------------------------------------------------------------------
void handleREAD() {
    LOCK();
    double t = (CorF == 'F') ? (temp * 1.8 + 32.0) : temp;
    String msg = "0," + String(t, 1) + "," + String(t, 1) + "," +
                 String(sendBuffer[HEAT_BYTE]) + "," + String(sendBuffer[VENT_BYTE]) + "\n";
    UNLOCK();
    notifyNimBLEClient(msg);
}

void handleCHAN() {
    notifyNimBLEClient("# Active channels set to 2100\n");
}

// -----------------------------------------------------------------------------
// Phan tich lenh tu HiBean
// -----------------------------------------------------------------------------
void parseAndExecuteCommands(String input) {
    input.trim();
    input.toUpperCase();
    g_lastHiBeanMs = millis();

    int split1 = input.indexOf(';');
    String command = "", param = "", subcommand = "";

    if (split1 >= 0) {
        command = input.substring(0, split1);
        String remainder = input.substring(split1 + 1);
        int split2 = remainder.indexOf(';');
        if (split2 >= 0) {
            subcommand = remainder.substring(0, split2);
            param = remainder.substring(split2 + 1);
        } else {
            param = remainder;
        }
    } else {
        command = input;
    }

    if (command == "PID") {
        if (param == "ON") {
            setPIDMode(true);
        } else if (param == "OFF") {
            setPIDMode(false);
        } else if (subcommand == "SV") {
            double sv = param.toDouble();
            if (CorF == 'F') sv = (sv - 32.0) / 1.8;      // noi bo luon la do C
            if (sv > 0 && sv <= 300) { LOCK(); pSetpoint = sv; UNLOCK(); }
        } else if (subcommand == "T") {
            double v[3] = { myPIDConfig.getKp(), myPIDConfig.getKi(), myPIDConfig.getKd() };
            int n = 0;
            while (param.length() > 0 && n < 3) {
                int index = param.indexOf(';');
                if (index == -1) { v[n++] = param.toDouble(); break; }
                v[n++] = param.substring(0, index).toDouble();
                param = param.substring(index + 1);
            }
            LOCK();
            myPIDConfig.setKp(v[0]); myPIDConfig.setKi(v[1]); myPIDConfig.setKd(v[2]);
            myPIDConfig.apply(myPID);
            UNLOCK();
        } else if (subcommand == "PM") {
            LOCK();
            myPIDConfig.setPMode(param == "M" ? P_ON_M : P_ON_E);
            myPIDConfig.apply(myPID);
            UNLOCK();
        } else if (subcommand == "CT") {
            LOCK();
            myPIDConfig.setSampleTime(param.toInt());
            myPIDConfig.apply(myPID);
            UNLOCK();
        }
    } else if (command == "OT1") {
        handleOT1(param.toInt(), SRC_HIBEAN);
    } else if (command == "READ") {
        handleREAD();
    } else if (command == "OT2") {
        handleVENT(param.toInt());
    } else if (command == "OFF") {
        roasterShutdown();
    } else if (command == "ESTOP") {
        eStop();
    } else if (command == "DRUM") {
        handleDRUM(param.toInt());
    } else if (command == "FILTER") {
        handleFILTER(param.toInt());
    } else if (command == "COOL") {
        handleCOOL(param.toInt());
    } else if (command == "CHAN") {
        handleCHAN();
    } else if (command == "UNITS") {
        if (split1 >= 0) {
            char u = input.charAt(split1 + 1);
            if (u == 'C' || u == 'F') CorF = u;
        }
    }
}

// -----------------------------------------------------------------------------
// Phat khung dieu khien xuong may rang (bit-bang, ~100 ms/khung, chi goi tu loop())
// -----------------------------------------------------------------------------
static void pulsePin(int pin, int duration) {
    if (pin < 0) return;
    digitalWrite(pin, LOW);
    delayMicroseconds(duration);
    digitalWrite(pin, HIGH);
}

void sendRoasterFrame(const uint8_t *buf) {
    pulsePin(TX_PIN, START_PULSE);
    delayMicroseconds(START_DELAY);

    for (int i = 0; i < CONTROLLER_LENGTH; i++) {
        for (int j = 0; j < 8; j++) {
            pulsePin(TX_PIN, bitRead(buf[i], j) ? 1500 : PULSE_ZERO);
            delayMicroseconds(POST_PULSE_DELAY);
        }
    }
}

void sendRoasterMessage() {
    uint8_t buf[CONTROLLER_LENGTH];
    LOCK();
    memcpy(buf, sendBuffer, CONTROLLER_LENGTH);
    UNLOCK();
    sendRoasterFrame(buf);
}
