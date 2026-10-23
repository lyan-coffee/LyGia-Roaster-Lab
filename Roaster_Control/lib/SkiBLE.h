/*
 * GPL v3 - xem LyGia_Roaster.ino
 */
#pragma once
#include <NimBLEDevice.h>
#include <queue>
#include <string>
#include "SkiSync.h"
#include "SkiPIDConfig.h"


// -----------------------------------------------------------------------------
// UUIDs
// -----------------------------------------------------------------------------
#define SERVICE_UUID           "6e400001-b5a3-f393-e0a9-e50e24dcca9e" // NUS service
#define CHARACTERISTIC_UUID_RX "6e400002-b5a3-f393-e0a9-e50e24dcca9e" // Write
#define CHARACTERISTIC_UUID_TX "6e400003-b5a3-f393-e0a9-e50e24dcca9e" // Notify

#define PID_TUNE          "6dbf0201-758d-4b5e-bc11-40cfaea42dfe" // ppp.pp,iii.ii,ddd.dd
#define PID_MODE          "6dbf0202-758d-4b5e-bc11-40cfaea42dfe" // "P_ON_M" | "P_ON_E"
#define PID_SAMPLE_TIME   "6dbf0203-758d-4b5e-bc11-40cfaea42dfe" // iiii (ms)
#define PID_MAX_POWER     "6dbf0204-758d-4b5e-bc11-40cfaea42dfe" // 0-100 (%)

// -----------------------------------------------------------------------------
// Globals
// -----------------------------------------------------------------------------
NimBLEServer* pServer = nullptr;
NimBLECharacteristic* pTxCharacteristic = nullptr;

volatile bool deviceConnected = false;            // HiBean dang ket noi
volatile bool hibeanWanted    = (HIBEAN_DEFAULT_ON); // nguoi dung bat/tat (nut HiBean tren man hinh)
static bool     bleActive     = false;            // trang thai thuc te cua quang ba
static uint16_t g_connHandle  = 0;

extern String firmWareVersion;
extern String sketchName;
extern std::queue<String> messageQueue;
extern PID myPID;
extern PIDConfig myPIDConfig;

// 0 = tat, 1 = dang cho HiBean ket noi, 2 = da ket noi
static int hibeanLinkState() {
    if (!hibeanWanted) return 0;
    return deviceConnected ? 2 : 1;
}

// -----------------------------------------------------------------------------
// Server callbacks
// -----------------------------------------------------------------------------
class MyServerCallbacks : public NimBLEServerCallbacks {
  void onConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo) override {
    deviceConnected = true;
    g_connHandle = connInfo.getConnHandle();
    g_lastHiBeanMs = millis();
    // min 15ms, max 30ms, latency 4, timeout 5s (khuyen nghi cua Apple)
    pServer->updateConnParams(connInfo.getConnHandle(), 12, 24, 4, 500);
    D_println("BLE: Client connected.");
  }
  void onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason) override {
    deviceConnected = false;
    D_println("BLE: Client disconnected.");
    if (hibeanWanted) pServer->getAdvertising()->start();
  }
};

// -----------------------------------------------------------------------------
// Lenh tu HiBean -> dua vao hang doi (loop xu ly)
// -----------------------------------------------------------------------------
class RoasterCallbacks : public NimBLECharacteristicCallbacks {
  void onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override {
    String rxValue = String(pCharacteristic->getValue().c_str());
    rxValue.trim();
    if (rxValue.length() > 0) {
      D_print("BLE Write Received: ");  D_println(rxValue);
      LOCK();
      if (messageQueue.size() < 32) messageQueue.push(rxValue);
      UNLOCK();
    }
  }
};

// -----------------------------------------------------------------------------
// PID config characteristics
// -----------------------------------------------------------------------------
class PIDTuneCallback : public NimBLECharacteristicCallbacks {
  void onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override {
    String rxValue = String(pCharacteristic->getValue().c_str());
    double v[3] = { myPIDConfig.getKp(), myPIDConfig.getKi(), myPIDConfig.getKd() };
    int n = 0;
    while (rxValue.length() > 0 && n < 3) {
        int index = rxValue.indexOf(',');
        if (index == -1) { v[n++] = rxValue.toDouble(); break; }
        v[n++] = rxValue.substring(0, index).toDouble();
        rxValue = rxValue.substring(index + 1);
    }
    LOCK();
    myPIDConfig.setKp(v[0]); myPIDConfig.setKi(v[1]); myPIDConfig.setKd(v[2]);
    myPIDConfig.apply(myPID);
    UNLOCK();
  }
  void onRead(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override {
      pCharacteristic->setValue(String(myPIDConfig.getKp()) + ',' + String(myPIDConfig.getKi()) + ',' + String(myPIDConfig.getKd()));
  }
};

class PIDModeCallback : public NimBLECharacteristicCallbacks {
  void onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override {
    String rxValue = String(pCharacteristic->getValue().c_str());
    LOCK();
    myPIDConfig.setPMode(rxValue == "P_ON_E" ? P_ON_E : P_ON_M);
    myPIDConfig.apply(myPID);
    UNLOCK();
  }
  void onRead(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override {
      pCharacteristic->setValue(myPIDConfig.getPMode() == P_ON_E ? "P_ON_E" : "P_ON_M");
  }
};

class PIDSampleTimeCallback : public NimBLECharacteristicCallbacks {
  void onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override {
    String rxValue = String(pCharacteristic->getValue().c_str());
    LOCK();
    myPIDConfig.setSampleTime(rxValue.toInt());
    myPIDConfig.apply(myPID);
    UNLOCK();
  }
  void onRead(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override {
      pCharacteristic->setValue(String(myPIDConfig.getSampleTime()));
  }
};

class PIDMaxPowerCallback : public NimBLECharacteristicCallbacks {
  void onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override {
    String rxValue = String(pCharacteristic->getValue().c_str());
    LOCK();
    myPIDConfig.setMaxPower(rxValue.toInt());
    myPIDConfig.apply(myPID);
    UNLOCK();
  }
  void onRead(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override {
      pCharacteristic->setValue(String(myPIDConfig.getMaxPower()));
  }
};

// -----------------------------------------------------------------------------
// Notify tra loi cho HiBean (goi tu loop, core 1)
// -----------------------------------------------------------------------------
void notifyNimBLEClient(const String& message) {
    delay(30); // de HiBean thay do tre giua write va notify
    if (deviceConnected && pTxCharacteristic) {
        pTxCharacteristic->setValue(message.c_str());
        pTxCharacteristic->notify();
    }
}

// -----------------------------------------------------------------------------
// Khoi tao BLE
// -----------------------------------------------------------------------------
void initBLE() {
    std::string NimBLEDeviceName = BLE_NAME;                 // LySetting.h
    NimBLEDevice::init(NimBLEDeviceName);

    pServer = NimBLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    NimBLEService* pService = pServer->createService(SERVICE_UUID);

    pTxCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID_TX, NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::READ
    );

    NimBLECharacteristic* pRxCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID_RX, NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR
    );
    pRxCharacteristic->setCallbacks(new RoasterCallbacks());

    // PID_TUNE
    NimBLECharacteristic* pidTuneCharacteristic = pService->createCharacteristic(
        PID_TUNE, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE
    );
    pidTuneCharacteristic->setCallbacks(new PIDTuneCallback());
    BLEDescriptor* pidTuneDescriptor = pidTuneCharacteristic->createDescriptor(PID_TUNE, NIMBLE_PROPERTY::READ);
    pidTuneDescriptor->setValue("PID Tune: p.pp,i.ii,d.dd");
    pidTuneCharacteristic->addDescriptor(pidTuneDescriptor);

    // PID_MODE
    NimBLECharacteristic* pidModeCharacteristic = pService->createCharacteristic(
        PID_MODE, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE
    );
    pidModeCharacteristic->setCallbacks(new PIDModeCallback());
    BLEDescriptor* pidModeDescriptor = pidModeCharacteristic->createDescriptor(PID_MODE, NIMBLE_PROPERTY::READ);
    pidModeDescriptor->setValue("PID Mode: P_ON_M | P_ON_E");
    pidModeCharacteristic->addDescriptor(pidModeDescriptor);

    // PID_SAMPLE_TIME
    NimBLECharacteristic* pidSampletimeCharacteristic = pService->createCharacteristic(
        PID_SAMPLE_TIME, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE
    );
    pidSampletimeCharacteristic->setCallbacks(new PIDSampleTimeCallback());
    BLEDescriptor* pidSampleTimeDescriptor = pidSampletimeCharacteristic->createDescriptor(PID_SAMPLE_TIME, NIMBLE_PROPERTY::READ);
    pidSampleTimeDescriptor->setValue("PID Sample Time: iiii (ms)");
    pidSampletimeCharacteristic->addDescriptor(pidSampleTimeDescriptor);

    // PID_MAX_POWER
    NimBLECharacteristic* pidMaxPowerCharacteristic = pService->createCharacteristic(
        PID_MAX_POWER, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE
    );
    pidMaxPowerCharacteristic->setCallbacks(new PIDMaxPowerCallback());
    NimBLEDescriptor* pidMaxPowerDescriptor = pidMaxPowerCharacteristic->createDescriptor(PID_MAX_POWER, NIMBLE_PROPERTY::READ);
    pidMaxPowerDescriptor->setValue("PID Max Power: 0-100 (%)");
    pidMaxPowerCharacteristic->addDescriptor(pidMaxPowerDescriptor);

    pService->start();

    // Thong tin thiet bi cho HiBean
    NimBLEService* devInfoService = pServer->createService("180A");
    NimBLECharacteristic* boardCharacteristic = devInfoService->createCharacteristic("2A29", NIMBLE_PROPERTY::READ);
      boardCharacteristic->setValue(ARDUINO_BOARD);
    NimBLECharacteristic* sketchNameCharacteristic = devInfoService->createCharacteristic("2A28", NIMBLE_PROPERTY::READ);
      sketchNameCharacteristic->setValue(sketchName);
    NimBLECharacteristic* firmwareCharacteristic = devInfoService->createCharacteristic("2A26", NIMBLE_PROPERTY::READ);
      firmwareCharacteristic->setValue(sketchName + " " + firmWareVersion);
    devInfoService->start();

    // Quang ba: CHI mot goi (khong Scan Response) = co (3 byte) + ten DAY DU (toi da 26 ky tu). Khong dua ten ngan vao goi rieng:
    // app HiBean / Windows xem goi chinh va Scan Response nhu hai thiet bi khac nhau -> hien 2 ten ("LyGia Ro" + "LyGia Roaster").
    // (Ban dau setName() nhet ten 27 ky tu vao goi chinh -> vuot 31 byte, bi bo qua, thiet bi khong co ten.)
    NimBLEAdvertising* pAdvertising = pServer->getAdvertising();
    pAdvertising->enableScanResponse(false);
    {
        NimBLEAdvertisementData advData;
        advData.setFlags(BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP);
        advData.setName(NimBLEDeviceName.substr(0, 26), NimBLEDeviceName.size() <= 26);
        if (!pAdvertising->setAdvertisementData(advData)) D_println("BLE: setAdvertisementData loi");
    }
    if (hibeanWanted) pAdvertising->start();
    bleActive = hibeanWanted;
}

// -----------------------------------------------------------------------------
// Ap dung bat/tat HiBean (goi tu loop, core 1 - tranh goi NimBLE tu luong UI)
// -----------------------------------------------------------------------------
void bleService() {
    if (hibeanWanted == bleActive || !pServer) return;
    if (hibeanWanted) {
        pServer->getAdvertising()->start();
    } else {
        pServer->getAdvertising()->stop();
        if (deviceConnected) pServer->disconnect(g_connHandle);
    }
    bleActive = hibeanWanted;
}
