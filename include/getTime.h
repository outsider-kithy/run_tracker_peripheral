#pragma once
#include <M5Unified.h>
#include <time.h>
#include <stdlib.h>
#include "getGps.h"

// M5 StickS3の場合、GPSで時刻を同期

extern String startDate, endDate;
extern int elapsed;

time_t startTime = 0;
time_t endTime = 0;

uint32_t startMillis = 0;
uint32_t endMillis = 0;

String startDate, endDate;
int elapsed = 0;

time_t gpsUnix = 0;

bool isLeapYear(int year) {
    return (year % 4 == 0 && year % 100 != 0) ||
           (year % 400 == 0);
}

int daysInMonth(int year, int month) {
    static const int days[] = {
        31, 28, 31, 30, 31, 30,
        31, 31, 30, 31, 30, 31
    };

    if (month == 2 && isLeapYear(year)) {
        return 29;
    }

    return days[month - 1];
}

time_t dateTimeToUnixUtc(
    int year,
    int month,
    int day,
    int hour,
    int minute,
    int second
) {
    int64_t days = 0;

    for (int y = 1970; y < year; y++) {
        days += isLeapYear(y) ? 366 : 365;
    }

    for (int m = 1; m < month; m++) {
        days += daysInMonth(year, m);
    }

    days += day - 1;

    int64_t unixTime =
        days * 86400LL +
        hour * 3600LL +
        minute * 60LL +
        second;

    return static_cast<time_t>(unixTime);
}
// GPSからUnix時刻を取得する関数
time_t getGpsUnixTime() {
    if (!gps.date.isValid() || !gps.time.isValid()) {
        return false;
    }

    int year   = gps.date.year();
    int month  = gps.date.month();
    int day    = gps.date.day();
    int hour   = gps.time.hour();
    int minute = gps.time.minute();
    int second = gps.time.second();

    if (
        year < 2020 ||
        month < 1 || month > 12 ||
        day < 1 || day > daysInMonth(year, month) ||
        hour < 0 || hour > 23 ||
        minute < 0 || minute > 59 ||
        second < 0 || second > 60
    ) {
        return false;
    }

    time_t unixTime = dateTimeToUnixUtc(
        year,
        month,
        day,
        hour,
        minute,
        second
    );

	return unixTime;
}

// UNIXからyyyy-MM-dd HH:mm:ssの文字列に変換
String unixToString(time_t unixTime) {

    // UTC → JST
    time_t jst = unixTime + 9 * 3600;

    struct tm t;
    gmtime_r(&jst, &t);

    char buffer[32];

    snprintf(buffer,
             sizeof(buffer),
             "%04d-%02d-%02d %02d:%02d:%02d",
             t.tm_year + 1900,
             t.tm_mon + 1,
             t.tm_mday,
             t.tm_hour,
             t.tm_min,
             t.tm_sec);

    return String(buffer);
}

// 開始時刻を取得
void startTimer(){
	if (!getGpsUnixTime()) {
		Serial.println("GPS date/time is not ready");
		return;
	}

	startTime = getGpsUnixTime();
	Serial.print("Timer start time:");
	Serial.println(startTime);

	startMillis = millis();

	elapsed = 0;

	// startTimeをyyyy-MM-dd HH:mm:ssの文字列に変換
	startDate = unixToString(startTime);
}

// 終了時刻を取得
void stopTimer(){
		endTime = getGpsUnixTime();
		Serial.print("Timer stopped time:");
		Serial.println(endTime);

        endMillis = millis();

        elapsed = (endMillis - startMillis) / 1000;

		delay(100);

		// endTimeをyyyy-MM-dd HH:mm:ssの文字列に変換
		endDate = unixToString(endTime);

        Serial.printf(
            "Elapsed: %lld millseconds\n",
            static_cast<long long>(elapsed)
        );

		M5.Lcd.setCursor(150, 60);
		M5.Lcd.setTextColor(WHITE);
		M5.Lcd.print(elapsed);
		M5.Lcd.println("s");
}

