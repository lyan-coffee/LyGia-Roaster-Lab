/*
 * GPL v3 - xem LyGia_Roaster.ino
 *
 * Dong bo da luong: UI (LVGL) chay core 0, giao thuc may rang chay core 1 (loop),
 * BLE chay task rieng. Moi thao tac len trang thai may deu di qua mutex nay.
 */
#pragma once
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

static SemaphoreHandle_t g_mux = nullptr;
static volatile uint32_t g_lastHiBeanMs = 0;   // lan cuoi nhan lenh tu HiBean

static inline void syncInit() {
    if (!g_mux) g_mux = xSemaphoreCreateRecursiveMutex();
}

#define LOCK()   xSemaphoreTakeRecursive(g_mux, portMAX_DELAY)
#define UNLOCK() xSemaphoreGiveRecursive(g_mux)
