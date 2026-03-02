#pragma once
#include <M5Unified.h>
#include <TinyGPSPlus.h>
#include <vector>

extern HardwareSerial GPSserial;
extern TinyGPSPlus gps;
extern bool isRecording; // 状態管理
extern std::vector<std::pair<double, double>> path; // 軌跡記録
extern double totalDistance; // 総移動距離（m）

HardwareSerial GPSserial(2);
TinyGPSPlus gps;
bool isRecording = false;
std::vector<std::pair<double, double>> path;
double totalDistance = 0.0;

void setupGPS() {
  // GPSシリアル初期化
  GPSserial.begin(115200, SERIAL_8N1, 33, 32); // GPS

  delay(500);
}

void updateGPS(){

  static unsigned long lastSaveTime = 0;
  const unsigned long SAVE_INTERVAL = 60000;
 
  // GPSデータをTinyGPSPlusで解析
  while (GPSserial.available()) {
    gps.encode(GPSserial.read());
  }

  M5.update();

  // 現在位置が有効な場合
  if (gps.location.isValid()) {
    double lat = gps.location.lat();
    double lng = gps.location.lng();
    
    // Aボタン押下で録画開始/停止をトグル
    if (M5.BtnA.wasPressed()) {
      isRecording = !isRecording;

      if (isRecording) {
        path.clear();
        totalDistance = 0;
      }
    }

    // 録画中なら定期的に位置を保存
    if (isRecording && millis() - lastSaveTime > SAVE_INTERVAL) {
      lastSaveTime = millis();
      if (!path.empty()) {
        double prevLat = path.back().first;
        double prevLng = path.back().second;
        double dist = TinyGPSPlus::distanceBetween(prevLat, prevLng, lat, lng);
        totalDistance += dist;
      }
      path.push_back({lat, lng});
    }
  }
}

