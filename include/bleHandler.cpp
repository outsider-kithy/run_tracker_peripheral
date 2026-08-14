#include <M5Unified.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include "jsonHandler.h"

#define SERVICE_UUID "12345678-1234-5678-1234-56789abcdef0" // サービスUUID
#define TX_UUID "abcd1234-5678-90ab-cdef-1234567890ac" // 送信用キャラクタリステックのUUID
#define RX_UUID "98765432-1098-abcd-ef12-098765432109" // 受信用キャラクタリスティックのUUID

BLECharacteristic* txCharacteristic; // 送信用キャラクタリスティック
BLECharacteristic* rxCharacteristic; // 受信用キャラクタリスティック

static BLEServer* pServer = nullptr;

volatile bool ackReceived = false;
volatile bool syncRequested = false;
volatile bool deleteRequested = false;

// セントラルからの接続開始要求 = ACKコマンドを待ち受ける
void waitForAck() {
      unsigned long start = millis();
      while (!ackReceived) {
        if (millis() - start > 100) {  // 0.1秒タイムアウト
          Serial.println("ACK timeout");
          break;
        }
        delay(5);
      }
      ackReceived = false;
    }

// セントラルからのJSON同期要求 = SYNCコマンドに答えて、
// 保存してあるJSONファイルを送信する
void sendFileWithAck() {
  if(!LittleFS.begin(true)){
    Serial.println("LittleFS Mount Failed");
    return;
  }
  Serial.println("LittleFS Mounted!");

  File root = LittleFS.open("/");
  if (!root || !root.isDirectory()) {
    Serial.println("- failed to open directory");
    return;
  }

  File file = root.openNextFile();

  while (file) {

    if (file.isDirectory()) {
      file = root.openNextFile();   // 次のファイルへ（無限ループ防止）
      continue;
    } else {
      Serial.println(file.name());
      // ファイル内容をすべて表示
      while (file.available()) {
        String json = file.readString();
        // Serial.println(json);

        int totalLength = json.length();

        const int chunkSize = 120;

        for (int offset = 0; offset < totalLength; offset += chunkSize) {

          int currentChunkSize = min(chunkSize, totalLength - offset);

          uint8_t buffer[chunkSize];  // 最大120バイト

          // String → uint8_t配列へコピー
          memcpy(buffer, json.c_str() + offset, currentChunkSize);

          // セントラルにbufferをBLEで送信する
          txCharacteristic->setValue(buffer, currentChunkSize);
          txCharacteristic->notify();
          waitForAck();

          // デバッグ表示
        //   Serial.print("Chunk: ");
        //   Serial.write(buffer, currentChunkSize);
        //   Serial.println();
        }
      }
    }

    file.close();                    // 各ファイルごとにclose
    file = root.openNextFile();      // 次のファイル取得
  }

  Serial.println("All files listed.");
  root.close();

  // セントラルにALL_DONEコマンドを送る
  txCharacteristic->setValue("ALL_DONE");
  txCharacteristic->notify();
  waitForAck();
}

// セントラルからDELETEコマンドを受け取ってJSONファイルを削除する
void deleteAllJsonFiles() {

  if(deleteRequested){
    File root = LittleFS.open("/");
    File file = root.openNextFile();

    while (file) {
      String filename = String(file.name());
      file.close();

      if (!filename.startsWith("/")) {
        filename = "/" + filename;
      }

      if (filename.endsWith(".json")) {
        Serial.println("Deleting: " + filename);
        LittleFS.remove(filename);
      }

      file = root.openNextFile();
    }

  root.close();

  // セントラルにDELETE_DONEコマンドを送る
  txCharacteristic->setValue("DELETE_DONE");
  txCharacteristic->notify();
  deleteRequested = false;
  }
}

// 受信用キャラクタリスティックのコールバックを定義
class RxCharacteristicCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pChar) {
    std::string value = pChar->getValue();
	// セントラルからACKコマンドを受け取ったら
    if (value == "ACK") {
      ackReceived = true;
    }
	// セントラルからSYNCコマンドを受け取ったら
    if (value == "SYNC") {
      Serial.println("SYNC received");
      syncRequested = true;
    }
	// セントラルからDELETEコマンドを受け取ったら
    if (value == "DELETE") {
      Serial.println("DELETE received");
      deleteRequested = true;
    }
  }
};

// BLEの初期化
void initBLE() {
  BLEDevice::init("M5Stick Run Tracker");

  pServer = BLEDevice::createServer();

  BLEService* pService = pServer->createService(SERVICE_UUID);

  // TX (M5 → Flutter 通知専用)
  txCharacteristic =  pService->createCharacteristic(
    TX_UUID,
    BLECharacteristic::PROPERTY_NOTIFY
  );
  
  txCharacteristic->addDescriptor(new BLE2902());

  // RX (Flutter → M5 書き込み専用)
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

