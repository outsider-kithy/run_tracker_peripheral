#include <M5Unified.h>
#include <TinyGPSPlus.h>
#include <vector>
#include "getGps.h"

extern HardwareSerial GPSserial;
extern TinyGPSPlus gps;
// 軌跡記録
extern std::vector<std::pair<double, double>> path;
// 総移動距離（m）
extern double totalDistance; 
// 最後に保存した時間
extern unsigned long lastSaveTime;

extern void setupGPS();
extern void startGPS();
extern void updateGPS();
extern void stopGPS();

HardwareSerial GPSserial(2);
TinyGPSPlus gps;
std::vector<std::pair<double, double>> path;
double totalDistance = 0.0;

// 最後の保存時刻
unsigned long lastSaveTime = 0;

// 緯度・経度を書き込む間隔（例：1分）
const int SAVE_INTERVAL = 60000;

void setupGPS() {
  // GPSシリアル初期化
  GPSserial.begin(115200, SERIAL_8N1, 33, 32);
  // シリアル通信を初期化
  Serial.begin(9600);
  delay(500);

  Serial.println("GPS Start");

	while (GPSserial.available()) {
		Serial.print((char)GPSserial.read());
	}
  
}

void startGPS() {
    // GPSシリアルを常時読む
    while (GPSserial.available()) {
        gps.encode(GPSserial.read());
    }
	if (gps.location.isValid()) {
		double lat = gps.location.lat();
        double lng = gps.location.lng();
		Serial.print("Start Point:");
		Serial.print(lat, 6);
		Serial.print(",");
		Serial.println(lng, 6);
		path.push_back({lat, lng});
        lastSaveTime = millis();
	}
}

void updateGPS(){
	
	// GPSデータを更新
    while (GPSserial.available()) {
        gps.encode(GPSserial.read());
    }
	
    // 有効な位置情報がある場合
    if (gps.location.isValid()) {

		if (millis() - lastSaveTime >= SAVE_INTERVAL) {

            lastSaveTime = millis();

			double lat = gps.location.lat();
			double lng = gps.location.lng();

			if (path.empty()) {
				// 開始時刻を記録
				path.push_back({lat, lng});
				lastSaveTime = millis();
				return;
			} else if (!path.empty()) {
				// 以前の座標がある場合は距離計算
                double prevLat = path.back().first;
                double prevLng = path.back().second;

                double dist = TinyGPSPlus::distanceBetween(
                    prevLat,
                    prevLng,
                    lat,
                    lng
                );
                totalDistance += dist;
            }
			// Serial.print("Elapsed:");
			// Serial.println(lastSaveTime);
			// Serial.print(lat, 6);
			// Serial.print(",");
			// Serial.println(lng, 6);

			// 新しい座標を保存
			path.push_back({lat, lng});
        }
	}
}

void stopGPS(){
	M5.Lcd.setCursor(0, 80);
	M5.Lcd.setTextColor(WHITE);
	M5.Lcd.print(totalDistance);
	M5.Lcd.println(" m ");
	for (size_t i = 0; i < path.size(); i++) {

        Serial.print("{lat:");
        Serial.print(String(path[i].first, 6));

        Serial.print(", lng:");
        Serial.print(String(path[i].second, 6));

        Serial.println("}, ");
    }
}

