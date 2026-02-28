#include <vector>
#include <algorithm>
#include <M5Unified.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <LittleFS.h>
#include "getSteps.h"
#include "getTime.h"
#include "getGps.h"

#define SERVICE_UUID "12345678-1234-5678-1234-56789abcdef0"
#define TX_UUID "abcd1234-5678-90ab-cdef-1234567890ac"
#define RX_UUID "abcd1234-5678-90ab-cdef-1234567890ad"

BLECharacteristic* txCharacteristic;
BLECharacteristic* rxCharacteristic;

static BLEServer* pServer = nullptr;

volatile bool ackReceived = false;
volatile bool syncRequested = false;

void waitForAck() {

      unsigned long start = millis();

      while (!ackReceived) {
        if (millis() - start > 5000) {  // 5秒タイムアウト
          Serial.println("ACK timeout");
          break;
        }
        delay(5);
      }
      ackReceived = false;
    }

void sendFileWithAck(String path) {

      File file = LittleFS.open(path, "r");
      if (!file) {
        Serial.println("File open failed");
        return;
      }

      // FILEヘッダ送信
      txCharacteristic->setValue(("FILE:" + path + "\n").c_str());
      txCharacteristic->notify();
      waitForAck();

      const int chunkSize = 120;
      uint8_t buffer[chunkSize];

      while (file.available()) {

        int len = file.read(buffer, chunkSize);
        txCharacteristic->setValue(buffer, len);
        txCharacteristic->notify();

        waitForAck();
      }

      file.close();

      // EOF送信
      txCharacteristic->setValue("EOF\n");
      txCharacteristic->notify();
      waitForAck();
    }

    

    void sendAllJsonFiles() {

    File root = LittleFS.open("/");
    if (!root) {
      Serial.println("Failed to open root directory");
      return;
    }

    std::vector<String> jsonFiles;
    File file = root.openNextFile();

    while (file) {
      String filename = String(file.name());
      if (!filename.startsWith("/")) {
        filename = "/" + filename;
      }

      if (!file.isDirectory() && filename.endsWith(".json")) {
        jsonFiles.push_back(filename);
      }

      file.close();
      file = root.openNextFile();
    }
    root.close();

    std::sort(jsonFiles.begin(), jsonFiles.end());

    for (const auto& filename : jsonFiles) {
      Serial.printf("Sending: %s\n", filename.c_str());
      sendFileWithAck(filename);
    }

    txCharacteristic->setValue("ALL_DONE\n");
    txCharacteristic->notify();
    waitForAck();
  }

class RxCharacteristicCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pChar) {
    std::string value = pChar->getValue();
    if (value == "ACK") {
      ackReceived = true;
    }
    if (value == "SYNC") {
      Serial.println("SYNC received");
      syncRequested = true;
    }
  }
};


void initBLE() {
  BLEDevice::init("M5Stick Run Tracker");

  pServer = BLEDevice::createServer();

  BLEService* pService = pServer->createService(SERVICE_UUID);

  //
  // 🔹 TX (M5 → Flutter 通知専用)
  //
  txCharacteristic =  pService->createCharacteristic(
    TX_UUID,
    BLECharacteristic::PROPERTY_NOTIFY
  );
  
  txCharacteristic->addDescriptor(new BLE2902());

  //
  // 🔹 RX (Flutter → M5 書き込み専用)
  //
  rxCharacteristic = pService->createCharacteristic(
    RX_UUID,
    BLECharacteristic::PROPERTY_WRITE
  );
  rxCharacteristic->setCallbacks(new RxCharacteristicCallbacks());

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

 