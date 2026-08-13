#include <M5Unified.h>
#include "getSteps.h"
#include "getTime.h"
#include "getGps.h"
#include "bleHandler.h"
#include "manageBattery.h"

// Aボタンを押した回数
int pressCount = 0;

// ディスプレイをオフにする時間
unsigned long lastOperationMillis = 0;
const unsigned long SLEEP_TIMEOUT = 30000; // 30秒

void setup() {
  
  M5.begin();

  M5.Power.setExtOutput(true);
  delay(100);
  
  // バッテリー節約のため、画面の明るさを落とす
  M5.Display.setBrightness(32);
  // CPU周波数を160MHzに設定
  setCpuFrequencyMhz(160);

  // 起動時を「最後に操作した時刻」とする
  lastOperationMillis = millis();
  
  M5.Lcd.setRotation(3);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(0,0);
  M5.Lcd.println("Initializing...");

  // バッテリー残量を表示
  setBatteryCharge();

  //歩数カウントを初期化
  setupSteps();
 
  //GPSを初期化
  setupGPS();
  // BLE初期化
  initBLE(); 
  M5.Lcd.setCursor(0, 20);
  M5.Lcd.println("Press A button!");
}

void loop() {
  	M5.update();
	updateGPS();
	//電池残量を更新
	updateBatteryCharge();
	
	// 30秒でスリープモードに移行
	if (millis() - lastOperationMillis >= SLEEP_TIMEOUT) {
		previousSaveMillis = millis();
		displayOff();
	}

	// Aボタンが押された瞬間
	if (M5.BtnA.wasPressed()) {
		// 最終操作時刻を更新
    	lastOperationMillis = millis();
		displayOn();
		setBatteryCharge();

		// 押された回数を増やす
		pressCount++;
		// 1回目
		if (pressCount == 1) {
			M5.Lcd.setCursor(0, 40);
			M5.Lcd.setTextColor(YELLOW);
			M5.Lcd.println("Tracking Start!");
			//歩数カウントスタート
			startCountSteps();
			//タイマースタート
			startTimer();
			//GPSスタート
			startGPS();
		}

		// 2回目
		else if (pressCount == 2) {
			// 歩数カウントストップ
			stopCountSteps();
			// タイマーストップ
			stopTimer();
			// GPSストップ
			stopGPS();
			// BLEでデータ送信
			sendDataViaBLE(path, totalDistance, steps, elapsed);
			M5.Lcd.setCursor(0, 100);
			M5.Lcd.setTextColor(RED);
			M5.Lcd.println("Data was saved!");
		}
		// 3回目(画面とデータをリセット)
		else if (pressCount == 3) {

			pressCount = 0;

			M5.Lcd.fillScreen(BLACK);
			M5.Lcd.setTextColor(WHITE);
			M5.Lcd.setCursor(0,0);
			M5.Lcd.println("Initializing...");

			setBatteryCharge();

			path.clear();
			totalDistance = 0.0;
			steps = 0;
			elapsed = 0;
			startDate, endDate = "";

			M5.Lcd.setCursor(0, 20);
			M5.Lcd.println("Press A button!");
			
		}	
	}
}

