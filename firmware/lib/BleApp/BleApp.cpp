#include "BleApp.h"
#include <NimBLEDevice.h>
#include <string>

// Service and Characteristic UUIDs
#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define LIGHT_CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define FAN_CHARACTERISTIC_UUID "82563452-9477-4b78-953e-38ec6f43e592"

// New Config Characteristic UUIDs (Generated)
#define FAN_MIN_UUID "c0b9a304-1234-4567-89ab-cdef01234567"
#define FAN_MAX_UUID "c0b9a304-1234-4567-89ab-cdef01234568"
#define LIGHT_ON_UUID "c0b9a304-1234-4567-89ab-cdef01234569"
#define LIGHT_OFF_UUID "c0b9a304-1234-4567-89ab-cdef0123456a"

// Global callback storage
LightCallback g_lightCb = nullptr;
FanCallback g_fanCb = nullptr;
IntCallback g_fanMinCb = nullptr;
IntCallback g_fanMaxCb = nullptr;
IntCallback g_lightOnCb = nullptr;
IntCallback g_lightOffCb = nullptr;

NimBLEServer *pServer = nullptr;
// Characteristics
NimBLECharacteristic *pLightCharacteristic = nullptr;
NimBLECharacteristic *pFanCharacteristic = nullptr;
NimBLECharacteristic *pFanMinCharacteristic = nullptr;
NimBLECharacteristic *pFanMaxCharacteristic = nullptr;
NimBLECharacteristic *pLightOnCharacteristic = nullptr;
NimBLECharacteristic *pLightOffCharacteristic = nullptr;

class ServerCallbacks : public NimBLEServerCallbacks {
  void onConnect(NimBLEServer *pServer) {
    Serial.println("[BLE] Client connected");
    // Request connection parameter update for stability (Android friendly)
    // Min 15ms, Max 30ms, Latency 0, Timeout 400ms
    // Fixed: getPeerIDInfo is not available on NimBLEServer in this version, or
    // requires handling. Commented out to fix build.
    // pServer->updateConnParams(pServer->getPeerIDInfo(0).getConnHandle(), 12,
    // 24, 0, 40);
  };

  void onDisconnect(NimBLEServer *pServer) {
    Serial.println("[BLE] Client disconnected - Restarting advertising...");
    delay(500); // Give the stack time to clear the connection
    if (NimBLEDevice::getAdvertising()->start()) {
      Serial.println("[BLE] Advertising restarted.");
    } else {
      Serial.println("[BLE] Failed to restart advertising!");
    }
  }
};

class LightCallbacks : public NimBLECharacteristicCallbacks {
  void onWrite(NimBLECharacteristic *pCharacteristic) {
    std::string value = pCharacteristic->getValue();
    if (value.length() > 0 && g_lightCb) {
      bool on = (value[0] == 1);
      Serial.printf("[BLE] RX: Light -> %s\n", on ? "ON" : "OFF");
      g_lightCb(on);
    }
  }
};

class FanCallbacks : public NimBLECharacteristicCallbacks {
  void onWrite(NimBLECharacteristic *pCharacteristic) {
    std::string value = pCharacteristic->getValue();
    if (value.length() > 0 && g_fanCb) {
      uint8_t val = (uint8_t)value[0];
      Serial.printf("[BLE] RX: Fan -> %d%%\n", val);
      g_fanCb(val);
    }
  }
};

class IntCallbacks : public NimBLECharacteristicCallbacks {
  IntCallback callback;
  const char *name;

public:
  IntCallbacks(IntCallback cb, const char *n) : callback(cb), name(n) {}
  void onWrite(NimBLECharacteristic *pCharacteristic) {
    std::string value = pCharacteristic->getValue();
    if (value.length() > 0 && callback) {
      uint8_t val = (uint8_t)value[0];
      Serial.printf("[BLE] RX: Config %s -> %d\n", name, val);
      callback(val);
    }
  }
};

// Initialize BLE with all callbacks and initial values
void initBLE(LightCallback lightCb, FanCallback fanCb, IntCallback fanMinCb,
             IntCallback fanMaxCb, IntCallback lightOnCb,
             IntCallback lightOffCb, bool lightState, int fanVal, int fanMin,
             int fanMax, int lightOn, int lightOff) {

  g_lightCb = lightCb;
  g_fanCb = fanCb;
  g_fanMinCb = fanMinCb;
  g_fanMaxCb = fanMaxCb;
  g_lightOnCb = lightOnCb;
  g_lightOffCb = lightOffCb;

  NimBLEDevice::init("TOWER");

  // 1. Boost Power to Max (ESP_PWR_LVL_P9 = +9dBm)
  NimBLEDevice::setPower(ESP_PWR_LVL_P9);

  // Optional: Set Security to No bonding/encryption for simplicity
  NimBLEDevice::setSecurityAuth(false, false, false);

  pServer = NimBLEDevice::createServer();
  pServer->setCallbacks(new ServerCallbacks());

  NimBLEService *pService = pServer->createService(SERVICE_UUID);

  // --- Control ---
  // Light
  pLightCharacteristic = pService->createCharacteristic(
      LIGHT_CHARACTERISTIC_UUID,
      NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE);
  pLightCharacteristic->setCallbacks(new LightCallbacks());
  uint8_t initLight = lightState ? 1 : 0;
  pLightCharacteristic->setValue(&initLight, 1);

  // Fan
  pFanCharacteristic = pService->createCharacteristic(
      FAN_CHARACTERISTIC_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE);
  pFanCharacteristic->setCallbacks(new FanCallbacks());
  uint8_t initFan = (uint8_t)fanVal;
  pFanCharacteristic->setValue(&initFan, 1);

  // --- Config ---
  // Fan Min
  pFanMinCharacteristic = pService->createCharacteristic(
      FAN_MIN_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE);
  pFanMinCharacteristic->setCallbacks(new IntCallbacks(g_fanMinCb, "FanMin"));
  uint8_t initFanMin = (uint8_t)fanMin;
  pFanMinCharacteristic->setValue(&initFanMin, 1);

  // Fan Max
  pFanMaxCharacteristic = pService->createCharacteristic(
      FAN_MAX_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE);
  pFanMaxCharacteristic->setCallbacks(new IntCallbacks(g_fanMaxCb, "FanMax"));
  uint8_t initFanMax = (uint8_t)fanMax;
  pFanMaxCharacteristic->setValue(&initFanMax, 1);

  // Light On Hour
  pLightOnCharacteristic = pService->createCharacteristic(
      LIGHT_ON_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE);
  pLightOnCharacteristic->setCallbacks(
      new IntCallbacks(g_lightOnCb, "LightOnHour"));
  uint8_t initLightOn = (uint8_t)lightOn;
  pLightOnCharacteristic->setValue(&initLightOn, 1);

  // Light Off Hour
  pLightOffCharacteristic = pService->createCharacteristic(
      LIGHT_OFF_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE);
  pLightOffCharacteristic->setCallbacks(
      new IntCallbacks(g_lightOffCb, "LightOffHour"));
  uint8_t initLightOff = (uint8_t)lightOff;
  pLightOffCharacteristic->setValue(&initLightOff, 1);

  pService->start();

  NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();

  NimBLEAdvertisementData advData;
  advData.setFlags(0x06); // General Discoverable + BR/EDR Not Supported
  advData.setCompleteServices(NimBLEUUID(SERVICE_UUID));
  pAdvertising->setAdvertisementData(advData);

  NimBLEAdvertisementData scanData;
  scanData.setName("TOWER");
  pAdvertising->setScanResponseData(scanData);

  // 2. Faster Advertising Intervals
  // Min: 32 * 0.625ms = 20ms
  // Max: 64 * 0.625ms = 40ms
  pAdvertising->setMinInterval(32);
  pAdvertising->setMaxInterval(64);

  // Preferred params for Android (15ms - 30ms)
  // setMinPreferred not available in this NimBLE version
  // pAdvertising->setMinPreferred(0x0C);
  // pAdvertising->setMaxPreferred(0x18);

  pAdvertising->start();

  Serial.println("[BLE] Service started (High Power, Fast Adv).");
}
