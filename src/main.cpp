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
  M5.begin();
  M5.Lcd.println("Initializing...");

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
        M5.Lcd.println("Sending via BLE...");
        currentState = SENDING;
      }
      break;

    case SENDING:
      sendDataViaBLE(path, totalDistance, steps, elapsedSeconds);
      M5.Lcd.println("Data sent via BLE!");
      path.clear();
      currentState = DONE;
      break;

    case DONE:
      break;
  }
}
