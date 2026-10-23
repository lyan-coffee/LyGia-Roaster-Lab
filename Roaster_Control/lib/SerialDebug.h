/*
 * GPL v3 - xem LyGia_Roaster.ino
 */
#pragma once

// -----------------------------------------------------------------------------
// SERIAL_DEBUG = 1: in log ra Serial (can bat "USB CDC On Boot = Enabled").
// Khi debug, chan giao tiep may rang bi vo hieu hoa (TX/RX = -1).
// -----------------------------------------------------------------------------
#define SERIAL_DEBUG 0

// Chan giao tiep voi may rang - giong SkiPinDefns.h (ESP32-S3-Zero): TX 19, RX 20
// Luu y: GPIO 19/20 la chan USB native. Sau khi firmware chiem 2 chan nay (sau ROASTER_BOOT_DELAY_MS)
// cong USB khong con dung de nap/serial; muon nap lai: giu nut BOOT + RESET (hoac cam lai nguon).

#if SERIAL_DEBUG == 1
  static const int TX_PIN = -1;
  static const int RX_PIN = -1;
  #define D_print(...)    Serial.print(__VA_ARGS__)
  #define D_println(...)  Serial.println(__VA_ARGS__)
#else
  static const int TX_PIN = ROASTER_TX_PIN;
  static const int RX_PIN = ROASTER_RX_PIN;
  #define D_print(...)
  #define D_println(...)
#endif
