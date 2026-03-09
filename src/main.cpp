#include <LittleFS.h>
#include <M5Unified.h>
#include "ble_handler.h"

enum State {
  INIT,
  WAIT_FOR_START,
  TRACKING,
  SENDING,
  DONE
};

State currentState = INIT;

void setup() {

  Serial.begin(115200);
  
  M5.begin();
  M5.Lcd.setRotation(3);
  M5.Lcd.setTextSize(2);
  M5.Lcd.println("Initializing...");

  listFilesFromRoot();
  setupGPS();
  setupSteps();
  setupTime();
  initBLE(); // BLE初期化

  M5.Lcd.println("Press A button!");
  currentState = WAIT_FOR_START;
}

void loop() {
  M5.update();

  switch (currentState) {
    case WAIT_FOR_START:
      if (M5.BtnA.wasPressed()) {
        M5.Lcd.setTextColor(YELLOW);
        M5.Lcd.println("Tracking started!");
        isRecording = true;
        currentState = TRACKING;
      }
      break;

    case TRACKING:
      updateGPS();
      updateSteps();
      updateTime();

      if (M5.BtnA.wasPressed()) {
        isRecording = false;
        M5.Lcd.setTextColor(WHITE);
        M5.Lcd.println("Saving data...");
        currentState = SENDING;
      }
      break;

    case SENDING:
      saveRunDataToFile(path, totalDistance, steps, elapsedSeconds);
      M5.Lcd.setTextColor(RED);
      M5.Lcd.println("Data was saved!");
      path.clear();
      currentState = DONE;
      break;

    case DONE:
      break;
  }

   if (syncRequested) {
        syncRequested = false;
        txCharacteristic->setValue("READY");
        txCharacteristic->notify();
        delay(100);
        sendFileWithAck();
    }
    if(deleteRequested){
      txCharacteristic->setValue("DELETE_processing...");
      txCharacteristic->notify();
      delay(100);
      deleteAllJsonFiles();
    }
}
