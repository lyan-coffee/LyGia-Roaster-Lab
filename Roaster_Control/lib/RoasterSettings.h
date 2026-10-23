/*
 * GPL v3 - xem LyGia_Roaster.ino
 *
 * MODULE CAI DAT: luu huong man hinh, don vi nhiet do, ngon ngu, muc canh bao, bat/tat man hinh cho.
 *
 *  ENABLE_SETTINGS_PAGE = 1 : co nut banh rang + trang Setting, gia tri luu vao bo nho (NVS)
 *  ENABLE_SETTINGS_PAGE = 0 : khong co trang Setting; dung co dinh DEFAULT_* ben duoi
 */
#pragma once
#include <Arduino.h>
#include <Preferences.h>


// Gia tri mac dinh: lib/UIConfig.h (Phan 5, 6). Khoang muc canh bao: LySetting.h (WARN_TEMP_*).

enum : uint8_t { ORI_PORTRAIT = 0, ORI_LANDSCAPE = 1 };
enum : uint8_t { UNIT_C = 0, UNIT_F = 1 };
enum : uint8_t { LANG_EN = 0, LANG_VI = 1, LANG_ZH = 2, LANG_COUNT = 3 };

struct RoasterSettings {
    uint8_t orient;
    uint8_t unit;
    uint8_t lang;
    uint16_t warn;     // muc nhiet canh bao (do C)
    uint8_t idle;      // 1 = cho phep man hinh cho, 0 = tat
};

static uint16_t warn_clamp(int v) {
    v = ((v + WARN_TEMP_STEP / 2) / WARN_TEMP_STEP) * WARN_TEMP_STEP;
    if (v < WARN_TEMP_MIN) v = WARN_TEMP_MIN;
    if (v > WARN_TEMP_MAX) v = WARN_TEMP_MAX;
    return (uint16_t)v;
}

static RoasterSettings g_set = { DEFAULT_ORIENTATION, DEFAULT_TEMP_UNIT, DEFAULT_LANGUAGE,
                                 (uint16_t)(DEFAULT_WARN_TEMP < WARN_TEMP_MIN ? WARN_TEMP_MIN : (DEFAULT_WARN_TEMP > WARN_TEMP_MAX ? WARN_TEMP_MAX : DEFAULT_WARN_TEMP)),
                                 (uint8_t)(DEFAULT_IDLE_SCREEN ? 1 : 0) };


// ---- PID (Setting): bo mau + gia tri tuy chinh. Ap vao myPIDConfig / myPID (SkiCMD.h) va luu NVS ("pkp","pki","pkd","ppm") ----
#if ENABLE_SETTINGS_PAGE && ENABLE_PID_SETTINGS
struct PidPreset { const char *name; float kp, ki, kd; uint8_t pm; };       // pm: 0 = P tren sai so (P_ON_E), 1 = P tren do (P_ON_M)
// 3 bo CO DINH (nut Soft / Balanced / Fast)
static const PidPreset PID_PRESETS[3] = {
    { "Soft",     8.0f, 0.20f, 10.0f, 0 },     // em: tang vua phai, ra it vot, hop khi cam bien nhieu
    { "Balanced", 12.0f, 0.30f, 20.0f, 0 },    // can bang tang nhanh / vot qua Set
    { "Fast",     (float)PID_DEFAULT_KP, (float)PID_DEFAULT_KI, (float)PID_DEFAULT_KD, (uint8_t)(PID_DEFAULT_PMODE ? 0 : 1) },   // nhanh + phanh som = bo mac dinh (LySetting.h)
};
// 3 bo CUA NGUOI DUNG (nut P1 / P2 / P3): bam vao de sua, xac nhan Save moi luu (NVS "u1kp"... "u3pm")
static PidPreset g_pidUser[3] = {
    { "P1",  9.0f, 0.30f,  2.5f, 1 },          // mac dinh P1 = PID cu cua v2.7.0 (Classic)
    { "P2", 14.0f, 0.30f, 25.0f, 0 },
    { "P3", 20.0f, 0.40f, 40.0f, 0 },
};
static void pidCfgApply(float kp, float ki, float kd, uint8_t pm) {
    LOCK();
    myPIDConfig.setKp(kp); myPIDConfig.setKi(ki); myPIDConfig.setKd(kd);
    myPIDConfig.setPMode(pm ? P_ON_M : P_ON_E);
    myPIDConfig.apply(myPID);
    UNLOCK();
}
static void pidSettingsSave() {
    Preferences p;
    if (p.begin("lygia", false)) {
        p.putFloat("pkp", (float)myPIDConfig.getKp()); p.putFloat("pki", (float)myPIDConfig.getKi());
        p.putFloat("pkd", (float)myPIDConfig.getKd()); p.putUChar("ppm", myPIDConfig.getPMode() == P_ON_M ? 1 : 0);
        p.end();
    }
}
static void pidUserSave(int i) {
    Preferences p; char k[8];
    if (!p.begin("lygia", false)) return;
    snprintf(k, sizeof k, "u%dkp", i + 1); p.putFloat(k, g_pidUser[i].kp);
    snprintf(k, sizeof k, "u%dki", i + 1); p.putFloat(k, g_pidUser[i].ki);
    snprintf(k, sizeof k, "u%dkd", i + 1); p.putFloat(k, g_pidUser[i].kd);
    snprintf(k, sizeof k, "u%dpm", i + 1); p.putUChar(k, g_pidUser[i].pm);
    p.end();
}
static void pidSettingsLoad() {
    Preferences p;
    if (!p.begin("lygia", false)) return;
    for (int i = 0; i < 3; i++) {                                               // bo cua nguoi dung
        char k[8]; snprintf(k, sizeof k, "u%dkp", i + 1);
        if (!p.isKey(k)) continue;
        const float kp = p.getFloat(k, g_pidUser[i].kp);
        snprintf(k, sizeof k, "u%dki", i + 1); const float ki = p.getFloat(k, g_pidUser[i].ki);
        snprintf(k, sizeof k, "u%dkd", i + 1); const float kd = p.getFloat(k, g_pidUser[i].kd);
        snprintf(k, sizeof k, "u%dpm", i + 1); const uint8_t pm = p.getUChar(k, g_pidUser[i].pm);
        if (kp >= 0 && kp <= 50 && ki >= 0 && ki <= 2 && kd >= 0 && kd <= 100) { g_pidUser[i].kp = kp; g_pidUser[i].ki = ki; g_pidUser[i].kd = kd; g_pidUser[i].pm = pm ? 1 : 0; }
    }
    if (p.isKey("pkp")) {
        float kp = p.getFloat("pkp", 0), ki = p.getFloat("pki", 0), kd = p.getFloat("pkd", 0); uint8_t pm = p.getUChar("ppm", 0);
        p.end();
        if (kp >= 0 && kp <= 60 && ki >= 0 && ki <= 5 && kd >= 0 && kd <= 200) pidCfgApply(kp, ki, kd, pm);
    } else p.end();
}
#endif

static bool presetUnitIsF() { return g_set.unit == UNIT_F; }   // dung cho SkiCMD.h (moc PR)

static void settingsLoad() {
#if ENABLE_SETTINGS_PAGE
    Preferences p;
    if (p.begin("lygia", false)) {
        g_set.orient = p.getUChar("ori",  DEFAULT_ORIENTATION);
        g_set.unit   = p.getUChar("unit", DEFAULT_TEMP_UNIT);
        g_set.lang   = p.getUChar("lang", DEFAULT_LANGUAGE);
        g_set.warn   = p.getUShort("warn", DEFAULT_WARN_TEMP);
        g_set.idle   = p.getUChar("idle", DEFAULT_IDLE_SCREEN ? 1 : 0);
#if ENABLE_SIMULATION
        g_simScale = p.getUChar("simx", SIM_SCALE_DEFAULT);
#endif
        p.end();
    }
    if (g_set.orient > ORI_LANDSCAPE) g_set.orient = DEFAULT_ORIENTATION;
    if (g_set.unit   > UNIT_F)        g_set.unit   = DEFAULT_TEMP_UNIT;
    if (g_set.lang   >= LANG_COUNT)   g_set.lang   = DEFAULT_LANGUAGE;
    g_set.warn = warn_clamp(g_set.warn);
    if (g_set.idle > 1) g_set.idle = DEFAULT_IDLE_SCREEN ? 1 : 0;
#if ENABLE_PID_SETTINGS
    pidSettingsLoad();
#endif
#if ENABLE_SIMULATION
    if (g_simScale != 1 && g_simScale != 3 && g_simScale != 5 && g_simScale != 7) g_simScale = SIM_SCALE_DEFAULT;
#endif
#endif
}

static void settingsSave() {
#if ENABLE_SETTINGS_PAGE
    Preferences p;
    if (p.begin("lygia", false)) {
        p.putUChar("ori",  g_set.orient);
        p.putUChar("unit", g_set.unit);
        p.putUChar("lang", g_set.lang);
        p.putUShort("warn", g_set.warn);
        p.putUChar("idle", g_set.idle);
#if ENABLE_SIMULATION
        p.putUChar("simx", g_simScale);
#endif
        p.end();
    }
#endif
}
