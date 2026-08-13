#pragma once
#include "getGps.cpp"

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

