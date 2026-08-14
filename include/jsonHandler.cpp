#include <M5Unified.h>
#include <vector>
#include <algorithm>
#include <LittleFS.h>
#include "getSteps.h"
#include "getGps.h"
#include "getTime.h"

// 走行データをJSONに保存
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
  //JSONファイル保存
  file.print(json);
  file.close();
}

// ルートディレクトリにあるJSONファイルを一覧表示
void listFilesFromRoot(){
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
      file = root.openNextFile();   // ← 次のファイルへ（無限ループ防止）
      continue;
    } else {
      Serial.println(file.name());
    }
    file.close();
  }
  if(!file){
    Serial.println("No JSON Files.");
  }
  root.close();
}