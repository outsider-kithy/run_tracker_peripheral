#pragma once
#include <M5Unified.h>
#include "rtc_sync.h"

extern String startDate, endDate;
String startDate, endDate;

extern m5::rtc_time_t startTime, endTime;
m5::rtc_time_t startTime, endTime;

bool firstPress = false;
unsigned long startMillis = 0;
unsigned long endMillis = 0;
unsigned long elapsedSeconds = 0;
bool isTracking = false;

void setupTime() {
  syncRTCFromNTP(); // NTP→RTC同期
}

// rtc_time_tをStringに変換する関数
String rtcToString(const m5::rtc_date_t &d, const m5::rtc_time_t &t) {
  char buffer[32];
  // YYYY-MM-DD HH:MM:SS の形式に整形
  snprintf(buffer, sizeof(buffer), "%04d-%02d-%02d %02d:%02d:%02d",
           d.year, d.month, d.date, t.hours, t.minutes, t.seconds);
  return String(buffer);
}

int toSeconds(const m5::rtc_time_t& t) {
  return t.hours * 3600 + t.minutes * 60 + t.seconds;
}

void updateTime() {
  M5.update();

  if (M5.BtnA.wasReleased()) {

    if (!isTracking) {
      //開始時間をミリ秒として保存
      startMillis = millis();
      Serial.printf("START: %lu\n", startMillis);

      //開始時刻を文字列として保存
      startTime = M5.Rtc.getTime();
      startDate = rtcToString(M5.Rtc.getDate(), startTime);

      isTracking = true;

    } else {
      //終了時刻をミリ秒として保存
      endMillis = millis();
     
      //終了時刻を文字列として保存
      endTime = M5.Rtc.getTime();
      endDate = rtcToString(M5.Rtc.getDate(), endTime);

      //経過時間を計算
      elapsedSeconds = (endMillis - startMillis) / 1000;

      Serial.printf("Elapsed: %lu sec\n", elapsedSeconds);

      isTracking = false;
    }
  }

  delay(50);
}

