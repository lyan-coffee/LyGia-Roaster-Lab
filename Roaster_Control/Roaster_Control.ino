/*
 * This program is free software: you can redistribute it and/or modify
 * it under d:\Github\LyGia Roaster Lab\LyGia_Roaster\lib\UIConfig.hthe terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/***************************************************
 * LY GIA ROASTER - bang dieu khien + module HiBean (BLE)
 * Board  : Guition JC3248W535EN (ESP32-S3, 3.5" 320x480 AXS15231B QSPI)
 * Thu vien: GFX Library for Arduino, LVGL 9.x, NimBLE-Arduino 2.x, PID (br3ttb)
 *
 * Kien truc:
 *   core 1 (loop)  : giao thuc may rang (bit-bang) + lenh HiBean + PID + an toan
 *   core 0 (task)  : giao dien LVGL + cam ung
 *   BLE            : task NimBLE, chi day lenh vao hang doi
 *   Trang thai may : sendBuffer (lib/SkiCMD.h) - UI va HiBean cung doc/ghi
 ***************************************************/

#include <Arduino.h>
#include <PID_v1.h>

// ------------------------- CAU HINH -------------------------
// Moi thong so cau hinh nam trong LySetting.h (cung thu muc) va lib/UIConfig.h. Khong khai bao o day.
#include "LySetting.h"
// ------------------------------------------------------------

#include "lib/SerialDebug.h"
#include "lib/SkiSync.h"
#include "lib/SkiPIDConfig.h"
#include "lib/SkiParser.h"
#include "lib/SkiBLE.h"
#include "lib/SkiCMD.h"
#include "lib/RoasterSim.h"
#include "lib/RoasterGraphData.h"
#include "lib/RoasterProfileRun.h"
#include "lib/RoasterLog.h"
#include "lib/RoasterUI.h"

String firmWareVersion = String(FW_VERSION);          // FW_VERSION: LySetting.h
// Ten hien thi trong app HiBean (Device Settings > Firmware Version) = sketchName + " " + firmWareVersion -> "Ly Gia Roaster 2.7.0"
// (truoc day lay tu __FILE__ nen tren Windows hien ca duong dan D:\Arduino\...)
String sketchName = String(DEVICE_NAME);              // DEVICE_NAME: LySetting.h

// Nhiet do hat (LUON la do C)
double temp = 0.0;
char CorF = 'C';          // don vi HiBean yeu cau (chi dung khi tra loi READ)

SkyRoasterParser roaster;
std::queue<String> messageQueue;

double pInput, pOutput;
double pSetpoint = DEFAULT_PID_SETPOINT;   // do C
int manualHeatLevel = 0;

PIDConfig myPIDConfig;
PID myPID(&pInput, &pOutput, &pSetpoint,
          myPIDConfig.getKp(), myPIDConfig.getKi(), myPIDConfig.getKd(),
          myPIDConfig.getPMode(), DIRECT);

void setup() {
#if SERIAL_DEBUG
    Serial.begin(115200);
#endif
    syncInit();                       // phai truoc BLE va UI

    myPID.SetMode(MANUAL);
    myPID.SetOutputLimits(0.0, myPIDConfig.getMaxPower());
    myPID.SetSampleTime(myPIDConfig.getSampleTime());

    roasterShutdown();                // Heater 0%, moi thu tat

    startUI();                        // man hinh + cam ung tren core 0 (khong bi delay ben duoi chan)
    logInit();                        // gan phan vung FFat de luu log me rang (ham rong neu ENABLE_ROAST_LOG 0)
    initBLE();

    // Cho nap firmware xong roi moi chiem GPIO 19/20 (chan USB) lam TX/RX may rang
    delay(ROASTER_BOOT_DELAY_MS);

    if (TX_PIN >= 0) {
        pinMode(TX_PIN, OUTPUT);
        digitalWrite(TX_PIN, HIGH);
    }
    if (RX_PIN >= 0) {
        roaster.begin(RX_PIN);        // ISR gan tren core 1
        roaster.enableDebug(false);
    }
}

void loop() {
    // 1. bat/tat quang ba BLE theo nut HiBean tren man hinh
    bleService();

    // 2. nhiet do hat tu may rang
    if (roaster.msgAvailable()) {
        uint8_t msg[7];
        roaster.getMessage(msg);
        if (roaster.validate(msg)) {
            double t = roaster.getTemperature(msg);
            LOCK();
            temp = t;
            lastTempMs = millis();
            lastRealTempMs = lastTempMs;
            UNLOCK();
        }
    }

    // 2b. gia lap nhiet do (neu nguoi dung bat va khong co tin hieu that)
    simUpdate();

    // 2c. RoR (toc do tang nhiet) - lay mau moi giay
    rorUpdate();
    logTick();                                      // log me rang: chup trang thai me dang mo, luu khi END (ham rong neu ENABLE_ROAST_LOG 0)
    graphUpdate();                                  // du lieu bieu do rang (ham rong neu ENABLE_ROAST_GRAPH 0)
    profileRunTick();                               // Auto: chay luat cua Profile da chon (ham rong neu ENABLE_PROFILE_AUTO 0)

    // 3. lenh tu HiBean
    for (;;) {
        String msg;
        LOCK();
        bool has = !messageQueue.empty();
        if (has) { msg = messageQueue.front(); messageQueue.pop(); }
        UNLOCK();
        if (!has) break;
        parseAndExecuteCommands(msg);
    }

    // 4. an toan + PID
    safetyWatchdog();
    enforceInterlock();
    handlePIDControl();

    // 5. phat khung dieu khien xuong may rang (~100 ms, chan loop nhu ban goc)
    //    Khi gia lap dang chay: KHONG phat, de may that khong nhan lenh theo nhiet do gia
    if (g_simActive) {
        delay(5);
    } else {
        sendRoasterMessage();
        delay(1);
    }
}
