#include <M5Unified.h>

extern void setBatteryCharge();
extern void updateBatteryCharge();
extern void displayOff();
extern void displayOn();

// バッテリー残量を更新する頻度
unsigned long previousSaveMillis = 0;
const unsigned long BATTERY_INTERVAL = 120000; // 120秒

// バッテリー残量を取得して表示
void setBatteryCharge(){
    int batteryLevel = M5.Power.getBatteryLevel();
    if(batteryLevel > 50){
      M5.Lcd.setTextColor(WHITE);
    } else if(batteryLevel > 20) {
      M5.Lcd.setTextColor(YELLOW);
    } else {
      M5.Lcd.setTextColor(RED);
    }
	M5.Lcd.fillRect(140, 110, 240, 240, BLACK);
	M5.Lcd.setCursor(140, 110);
    M5.Lcd.print("Bat:");
    M5.Lcd.printf("%d%%", batteryLevel);
}

// バッテリー残量を更新
void updateBatteryCharge(){
	if (millis() - previousSaveMillis >= BATTERY_INTERVAL) {
		previousSaveMillis = millis();
		setBatteryCharge();
	}
}

// ディスプレイを消す
void displayOff(){
    M5.Lcd.setBrightness(0);
}

// ディスプレイを点ける
void displayOn(){
	M5.Lcd.setBrightness(32);
}

