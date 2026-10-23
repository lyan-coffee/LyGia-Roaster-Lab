/*
 * GPL v3 - xem LyGia_Roaster.ino
 *
 * CHAY PROFILE (AUTO): thuc hien cac luat (trigger) cua Profile da chon (g_prof_sel). Bat/tat: ENABLE_PROFILE_AUTO (LySetting.h).
 *  - Moi luat chi chay MOT LAN moi me: khi su kien xay ra (PreHeat, Charge, TP, DE, FC1s, Drop) hoac khi BT VUOT NGANG nguong (tu duoi len).
 *  - Luat chi DAT gia tri luc kich hoat, khong giu: nguoi dung van chinh tay Heater / Air / PID bat cu luc nao.
 *  - Luat dung cac ham panel (qua cac chan an toan nhu khi bam tay): Air thap, Drum OFF, Cool ON... chi canh bao, khong cat Heater.
 *  - TP luon tu nhan (gTpIdx). DE / FC1s tu danh dau neu profile bat autoDE / autoFC (va ENABLE_AUTO_MARKS = 1).
 *  - Goi profileRunTick() moi vong loop() (core 1), sau graphUpdate().
 */
#pragma once
#include "RoasterProfile.h"

#if ENABLE_PROFILE && ENABLE_PROFILE_AUTO && ENABLE_ROAST_GRAPH

#define PROF_MAX_RULES 16
static uint32_t profFired = 0;                 // bit i = luat i da chay trong me nay
static float    profPrevT = 0;                 // BT o vong truoc (de bat canh len khi vuot nguong)
static bool     profPrevOk = false;

static void profileRunReset() { profFired = 0; profPrevOk = false; }

static void profApply(const ProfRule &r) {
    // Air truoc, roi Heater / PID (de Air >= muc toi thieu khi bat PID)
    for (int k = 0; k < r.nact && k < 3; k++) if (r.act[k].type == PA_FAN) handleVENT(constrain((int)r.act[k].v, 0, 100));
    for (int k = 0; k < r.nact && k < 3; k++) {
        const ProfAction &a = r.act[k];
        switch (a.type) {
            case PA_PID_OFF: pidOffKeepHeat(); break;
            case PA_PID_ON:  pSetpoint = (double)a.v / 10.0; panelSetPID(true); break;
            case PA_HEATER:
                if (r.trig == PT_DROP) break;              // Drop da cat Heater (panelDrop); luat chi dat Air
                panelSetHeater((int)a.v);
                break;
            default: break;
        }
    }
}

static void profileRunTick() {
    LOCK();
    const int sel = g_prof_sel;
    if (sel < 0 || sel >= PROFILE_COUNT || !batchLive) {          // khong co profile / me chua mo: dat lai
        if (!batchLive) profFired = 0;
        profPrevOk = false;
        UNLOCK();
        return;
    }
    const ProfileDef &pf = PROFILE_FACTORY[sel];
    const bool tempOk = (lastTempMs != 0) && ((millis() - lastTempMs) < TEMP_SIGNAL_OK_MS);
    const float T = (float)temp;
    const bool tp = gTpIdx >= 0;

#if ENABLE_AUTO_MARKS
    if (batchRunning && tempOk && tp) {                             // tu danh dau DE, roi FC1s
        if (pf.autoDE && !(evDone & (1 << EV_DE)) && T >= pf.de.bt10 / 10.0f) panelEvent(EV_DE);
        if (pf.autoFC && (evDone & (1 << EV_DE)) && !(evDone & (1 << EV_FC1)) && T >= pf.fc.bt10 / 10.0f) panelEvent(EV_FC1);
    }
#endif

    for (int i = 0; i < pf.nRules && i < PROF_MAX_RULES; i++) {
        if (profFired & (1u << i)) continue;
        const ProfRule &r = pf.rules[i];
        bool go = false;
        switch (r.trig) {
            case PT_PREHEAT: go = (evDone & (1 << EV_PREHEAT)) != 0; break;
            case PT_CHARGE:  go = (evDone & (1 << EV_CHARGE))  != 0; break;
            case PT_TP:      go = tp; break;
            case PT_DE:      go = (evDone & (1 << EV_DE))      != 0; break;
            case PT_FC1S:    go = (evDone & (1 << EV_FC1))     != 0; break;
            case PT_DROP:    go = (evDone & (1 << EV_DROP))    != 0; break;
            case PT_BT_GT:   go = batchCharged && tempOk && profPrevOk && profPrevT <= r.val10 / 10.0f && T > r.val10 / 10.0f && (!r.afterTP || tp); break;
        }
        if (go) { profFired |= (1u << i); profApply(r); }
    }
    profPrevT = T; profPrevOk = tempOk;
    UNLOCK();
}

#else
static inline void profileRunReset() {}
static inline void profileRunTick() {}
#endif
