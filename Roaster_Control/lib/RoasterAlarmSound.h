/*
 * GPL v3 - xem LyGia_Roaster.ino
 *
 * AM THANH CANH BAO QUA NHIET (chay trong task rieng tren core 0, khong chan giao dien / may rang).
 *
 *  ALARM_AUDIO_MODE 0 : tat tieng (chi hien man hinh canh bao)
 *  ALARM_AUDIO_MODE 1 : loa qua mach khuech dai I2S tren bo (mac dinh)  <-- KIEM TRA CHAN TREN SO DO MACH
 *  ALARM_AUDIO_MODE 2 : coi (buzzer) noi chan GPIO ALARM_BUZZER_PIN
 *
 * Kieu keu: "bi-bi-bip" hai am (880 / 1320 Hz), nghi ~0.4 s, lap lai den khi tat.
 */
#pragma once
#include <Arduino.h>
#include "RoasterSettings.h"

// Chan loa, che do am thanh, am luong: LySetting.h muc 2 va 6.
#define ALARM_SAMPLE_RATE 16000

// Co dieu khien (doc boi task am thanh, ghi boi giao dien)
static volatile bool     g_alarm_sound      = false;   // true = dang keu
static volatile uint32_t g_alarm_test_until = 0;       // != 0: che do thu den moc millis() nay

#if ENABLE_OVERHEAT_ALARM && ALARM_AUDIO_MODE == 1
  #if __has_include("driver/i2s_std.h")
    #include "driver/i2s_std.h"
    #define ALARM_HAS_I2S 1
  #endif
#endif

// Mau am: moi o 100 ms, 0 = im lang
static const uint16_t ALARM_PATTERN[] = { 880, 880, 1320, 1320, 0, 0, 0, 0 };
static const int ALARM_PATTERN_LEN = sizeof(ALARM_PATTERN) / sizeof(ALARM_PATTERN[0]);

#if defined(ALARM_HAS_I2S)

static void alarm_audio_task(void *) {
    i2s_chan_handle_t tx = nullptr;
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
    if (i2s_new_channel(&chan_cfg, &tx, NULL) != ESP_OK) vTaskDelete(NULL);

    i2s_std_config_t std_cfg = {
        .clk_cfg  = I2S_STD_CLK_DEFAULT_CONFIG(ALARM_SAMPLE_RATE),
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO),
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,
            .bclk = (gpio_num_t)ALARM_I2S_BCLK,
            .ws   = (gpio_num_t)ALARM_I2S_LRCK,
            .dout = (gpio_num_t)ALARM_I2S_DOUT,
            .din  = I2S_GPIO_UNUSED,
            .invert_flags = { .mclk_inv = false, .bclk_inv = false, .ws_inv = false },
        },
    };
    if (i2s_channel_init_std_mode(tx, &std_cfg) != ESP_OK) vTaskDelete(NULL);

    static int16_t buf[ALARM_SAMPLE_RATE / 10];            // 100 ms
    const int N = ALARM_SAMPLE_RATE / 10;
    bool enabled = false;
    float phase = 0.0f;
    int pat = 0;

    for (;;) {
        if (!g_alarm_sound) {
            if (enabled) { i2s_channel_disable(tx); enabled = false; }
            vTaskDelay(pdMS_TO_TICKS(50));
            continue;
        }
        if (!enabled) { i2s_channel_enable(tx); enabled = true; pat = 0; phase = 0.0f; }

        const uint16_t f = ALARM_PATTERN[pat++ % ALARM_PATTERN_LEN];
        if (f == 0) {
            memset(buf, 0, sizeof(buf));
        } else {
            const float inc = 2.0f * PI * (float)f / (float)ALARM_SAMPLE_RATE;
            for (int i = 0; i < N; i++) {
                buf[i] = (int16_t)(sinf(phase) * ALARM_VOLUME);
                phase += inc;
                if (phase > 2.0f * PI) phase -= 2.0f * PI;
            }
        }
        size_t written = 0;
        i2s_channel_write(tx, buf, sizeof(buf), &written, 300);   // chan den khi DMA con cho -> tu dinh nhip
    }
}

#elif ENABLE_OVERHEAT_ALARM && ALARM_AUDIO_MODE == 2

static void alarm_audio_task(void *) {
    int pat = 0;
    bool on = false;
    for (;;) {
        if (!g_alarm_sound || ALARM_BUZZER_PIN < 0) {
            if (on) { noTone(ALARM_BUZZER_PIN); on = false; }
            vTaskDelay(pdMS_TO_TICKS(50));
            continue;
        }
        const uint16_t f = ALARM_PATTERN[pat++ % ALARM_PATTERN_LEN];
        if (f) { tone(ALARM_BUZZER_PIN, f); on = true; }
        else   { noTone(ALARM_BUZZER_PIN);  on = false; }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

#endif

// Goi mot lan khi khoi dong giao dien (trong ui_task, core 0)
static void alarmSoundStart() {
#if defined(ALARM_HAS_I2S) || (ENABLE_OVERHEAT_ALARM && ALARM_AUDIO_MODE == 2)
    xTaskCreatePinnedToCore(alarm_audio_task, "alarm", 4096, NULL, 1, NULL, 0);
#endif
}
