#pragma once
#include <M5Unified.h>
#include <time.h>
#include <stdlib.h>
#include "syncRtc.h"
#include "getGps.h"

extern m5::rtc_time_t startTime, endTime;
extern String startDate, endDate;
extern int elapsed;

m5::rtc_time_t startTime, endTime;
String startDate, endDate;
int elapsed = 0;

// rtc_time_tをStringに変換する関数
String rtcToString(const m5::rtc_date_t &d, const m5::rtc_time_t &t) {
  char buffer[32];
  // YYYY-MM-DD HH:MM:SS の形式に整形
  snprintf(buffer, sizeof(buffer), "%04d-%02d-%02d %02d:%02d:%02d",
           d.year, d.month, d.date, t.hours, t.minutes, t.seconds);
  return String(buffer);
}


//タイマーを初期化
void setupTimer() {
  syncRTCFromNTP();

}

//タイマースタート
void startTimer(){
	// M5 StickC Plus3の場合
	M5.update();
	startTime = M5.Rtc.getTime();
	startDate = rtcToString(M5.Rtc.getDate(), startTime);
}

//タイマーストップ
void stopTimer(){
	// M5 StickC Plus3の場合
	endTime = M5.Rtc.getTime();
	endDate = rtcToString(M5.Rtc.getDate(), endTime);

	//経過時間を計算
	elapsed = (endTime.hours - startTime.hours) * 3600 +
				(endTime.minutes - startTime.minutes) * 60 +
				(endTime.seconds - startTime.seconds);

	int hours = (int)(elapsed / 3600);
	int minutes = ((int)elapsed % 3600) / 60;
	int seconds = (int)elapsed % 60;
	M5.Lcd.setCursor(150, 60);
	M5.Lcd.setTextColor(WHITE);
	M5.Lcd.print(elapsed);
	M5.Lcd.print(" s, ");
}
