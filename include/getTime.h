#pragma once
#include <M5Unified.h>
#include "rtc_sync.h"

extern bool firstPress;
extern m5::rtc_time_t startTime, endTime;
extern String startDate, endDate;
extern int elapsedSeconds;

bool firstPress = false;
m5::rtc_time_t startTime, endTime;
String startDate, endDate;
int elapsedSeconds = 0;

// rtc_time_tをStringに変換する関数
String rtcToString(const m5::rtc_date_t &d, const m5::rtc_time_t &t) {
  char buffer[32];
  // YYYY-MM-DD HH:MM:SS の形式に整形
  snprintf(buffer, sizeof(buffer), "%04d-%02d-%02d %02d:%02d:%02d",
           d.year, d.month, d.date, t.hours, t.minutes, t.seconds);
  return String(buffer);
}

void setupTime() {
  syncRTCFromNTP(); // NTP→RTC同期
}

void updateTime() {
  M5.update();

  if (M5.BtnA.wasPressed()) {
    if (!firstPress) {
      startTime = M5.Rtc.getTime();
      startDate = rtcToString(M5.Rtc.getDate(), startTime);
      firstPress = true;

    } else {
      endTime = M5.Rtc.getTime();
      endDate = rtcToString(M5.Rtc.getDate(), endTime);
      
      //経過時間を計算
      int elapsed = (endTime.hours - startTime.hours) * 3600 +
                    (endTime.minutes - startTime.minutes) * 60 +
                    (endTime.seconds - startTime.seconds);

      int hours = (int)(elapsed / 3600);
      int minutes = ((int)elapsed % 3600) / 60;
      int seconds = (int)elapsed % 60;
      
      firstPress = false;
    }
  }

  delay(50);
}

