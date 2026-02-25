#include <M5Unified.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <LittleFS.h>
#include "getSteps.h"
#include "getTime.h"
#include "getGps.h"

#define SERVICE_UUID        "12345678-1234-5678-1234-56789abcdef0"
#define CHARACTERISTIC_UUID "abcd1234-5678-90ab-cdef-1234567890ab"

static BLEServer* pServer = nullptr;
static BLECharacteristic* pCharacteristic = nullptr;
static bool deviceConnected = false;

class MyCharacteristicCallbacks : public BLECharacteristicCallbacks {
  //存在するJSONファイル一覧をBLEで送信
  void sendFileList() {
    File root = LittleFS.open("/");
    File file = root.openNextFile();

    String list = "";

    while (file) {
      list += String(file.name()) + ",";
      file = root.openNextFile();
    }

    pCharacteristic->setValue(list.c_str());
    pCharacteristic->notify();
  }

  //JSONファイルをチャンクに分けて送信
  void sendFileInChunks(String filename) {

    File file = LittleFS.open(filename, "r");
    if (!file) return;

    const int chunkSize = 180; // 安全サイズ
    uint8_t buffer[chunkSize];

    while (file.available()) {
      int len = file.read(buffer, chunkSize);

      pCharacteristic->setValue(buffer, len);
      pCharacteristic->notify();

      delay(20); // BLE詰まり防止
    }

    file.close();

    // 送信終了マーカー
    pCharacteristic->setValue("EOF");
    pCharacteristic->notify();
  }

  void onWrite(BLECharacteristic* pCharacteristic) override {
    std::string value = pCharacteristic->getValue();
    String command = String(value.c_str());

    if (command == "LIST") {
      sendFileList();
    }
    else if (command.startsWith("GET:")) {
      String filename = command.substring(4);
      sendFileInChunks(filename);
    }
  }
};

void initBLE() {
  BLEDevice::init("M5Stick Run Tracker");

  pServer = BLEDevice::createServer();

  BLEService* pService = pServer->createService(SERVICE_UUID);

  pCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ | 
    BLECharacteristic::PROPERTY_NOTIFY | 
    BLECharacteristic::PROPERTY_WRITE
  );

  pCharacteristic->setCallbacks(new MyCharacteristicCallbacks());

  pCharacteristic->addDescriptor(new BLE2902());
  
  pService->start();

  BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  BLEDevice::startAdvertising();
}

void saveRunDataToFile(const std::vector<std::pair<double, double>>& path,
                       double totalDistance,
                       int steps,
                       int elapsedSeconds) {
  
  Serial.println("saveRunDataToFile called");

  // --- JSON生成 ---
  String json = "{";
  json += "\"points\":[";

  for (size_t i = 0; i < path.size(); i++) {
    json += String("{\"lat\":") + String(path[i].first, 6) +
            ",\"lng\":" + String(path[i].second, 6) + "}";
    if (i < path.size() - 1) json += ",";
  }

  json += "],";
  json += "\"distance\":" + String(totalDistance, 2) + ",";
  json += "\"steps\":" + String(steps) + ",";
  json += "\"elapsedSeconds\":" + String(elapsedSeconds) + ",";
  json += "\"startDate\":\"" + startDate + "\",";
  json += "\"endDate\":\"" + endDate + "\"";
  json += "}";

  Serial.println(json);

  // --- ファイル名生成（例：/run_1700000000.json） ---
  String filename = "/run_" + String(time(nullptr)) + ".json";

  File file = LittleFS.open(filename, "w");
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }

  Serial.println("saved as ");
  Serial.println(filename);

  file.print(json);
  file.close();
}

//JSONファイル一覧を表示
void listFiles() {
    File root = LittleFS.open("/");
    File file = root.openNextFile();

    Serial.println("---- File List ----");
    while (file) {
        Serial.println(file.name());
        file = root.openNextFile();
    }
    Serial.println("-------------------");
}

