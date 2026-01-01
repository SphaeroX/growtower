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
  void onConnect(NimBLEServer *pServer) { Serial.println("Client connected"); };

  void onDisconnect(NimBLEServer *pServer) {
    Serial.println("Client disconnected - Waiting to restart advertising...");
    delay(500); // Give the stack time to clear the connection
    if (NimBLEDevice::getAdvertising()->start()) {
      Serial.println("Advertising restarted successfully.");
    } else {
      Serial.println("Failed to restart advertising!");
    }
  }
};

class LightCallbacks : public NimBLECharacteristicCallbacks {
  void onWrite(NimBLECharacteristic *pCharacteristic) {
    std::string value = pCharacteristic->getValue();
    if (value.length() > 0 && g_lightCb) {
      g_lightCb(value[0] == 1);
    }
  }
};

class FanCallbacks : public NimBLECharacteristicCallbacks {
  void onWrite(NimBLECharacteristic *pCharacteristic) {
    std::string value = pCharacteristic->getValue();
    if (value.length() > 0 && g_fanCb) {
      g_fanCb(value[0]);
    }
  }
};

class IntCallbacks : public NimBLECharacteristicCallbacks {
  IntCallback callback;

public:
  IntCallbacks(IntCallback cb) : callback(cb) {}
  void onWrite(NimBLECharacteristic *pCharacteristic) {
    std::string value = pCharacteristic->getValue();
    if (value.length() > 0 && callback) {
      callback(value[0]);
    }
  }
};

void initBLE(LightCallback lightCb, FanCallback fanCb, IntCallback fanMinCb,
             IntCallback fanMaxCb, IntCallback lightOnCb,
             IntCallback lightOffCb) {

  g_lightCb = lightCb;
  g_fanCb = fanCb;
  g_fanMinCb = fanMinCb;
  g_fanMaxCb = fanMaxCb;
  g_lightOnCb = lightOnCb;
  g_lightOffCb = lightOffCb;

  NimBLEDevice::init("TOWER");

  pServer = NimBLEDevice::createServer();
  pServer->setCallbacks(new ServerCallbacks());

  NimBLEService *pService = pServer->createService(SERVICE_UUID);

  // --- Control ---
  // Light
  pLightCharacteristic = pService->createCharacteristic(
      LIGHT_CHARACTERISTIC_UUID,
      NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE);
  pLightCharacteristic->setCallbacks(new LightCallbacks());
  uint8_t dummy0 = 0;
  pLightCharacteristic->setValue(&dummy0, 1);

  // Fan
  pFanCharacteristic = pService->createCharacteristic(
      FAN_CHARACTERISTIC_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE);
  pFanCharacteristic->setCallbacks(new FanCallbacks());
  pFanCharacteristic->setValue(&dummy0, 1);

  // --- Config ---
  // Fan Min
  pFanMinCharacteristic = pService->createCharacteristic(
      FAN_MIN_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE);
  pFanMinCharacteristic->setCallbacks(new IntCallbacks(g_fanMinCb));
  pFanMinCharacteristic->setValue(&dummy0,
                                  1); // Should ideally set to current value

  // Fan Max
  pFanMaxCharacteristic = pService->createCharacteristic(
      FAN_MAX_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE);
  pFanMaxCharacteristic->setCallbacks(new IntCallbacks(g_fanMaxCb));
  uint8_t dummy100 = 100;
  pFanMaxCharacteristic->setValue(&dummy100, 1);

  // Light On Hour
  pLightOnCharacteristic = pService->createCharacteristic(
      LIGHT_ON_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE);
  pLightOnCharacteristic->setCallbacks(new IntCallbacks(g_lightOnCb));
  uint8_t dummy18 = 18;
  pLightOnCharacteristic->setValue(&dummy18, 1);

  // Light Off Hour
  pLightOffCharacteristic = pService->createCharacteristic(
      LIGHT_OFF_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE);
  pLightOffCharacteristic->setCallbacks(new IntCallbacks(g_lightOffCb));
  uint8_t dummy14 = 14;
  pLightOffCharacteristic->setValue(&dummy14, 1);

  pService->start();

  NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();

  NimBLEAdvertisementData advData;
  advData.setFlags(0x06); // General Discoverable + BR/EDR Not Supported
  advData.setCompleteServices(NimBLEUUID(SERVICE_UUID));
  pAdvertising->setAdvertisementData(advData);

  NimBLEAdvertisementData scanData;
  scanData.setName("TOWER");
  pAdvertising->setScanResponseData(scanData);

  pAdvertising->start();

  Serial.println("BLE Dedicated Service with Config started...");
}
