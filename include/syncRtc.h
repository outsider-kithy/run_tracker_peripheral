#pragma once
#include <M5Unified.h>
#include "time.h"
#include <WiFi.h>
#include <WiFiManager.h>

// NSV領域に保存されたWi-Fi接続情報を読み出す
// void ensureWiFiConfigured() {
//     WiFiManager wm;

//     if (!wm.autoConnect("M5-Setup")) {
//         ESP.restart();
//     }
// }

void syncRTCFromNTP() {
    WiFi.begin();
    unsigned long startAttempt = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < 5000) {
        delay(500);
    }
    if (WiFi.status() != WL_CONNECTED) {
        return;
    }
    // NTPを取得
    configTime(3600L * 9, 0, "ntp.nict.jp", "ntp.jst.mfeed.ad.jp");

    // 起動ごとに時刻同期するために同期タイミングを一時的に変更
    auto default_interval = sntp_get_sync_interval(); // ミリ秒単位
    sntp_set_system_time(1, 0);                       // 1秒

    // NTPとRTCを同期
    struct tm timeInfo;
    if (getLocalTime(&timeInfo))
    {
        M5.Rtc.setDateTime(timeInfo);
    }

    sntp_set_system_time(default_interval * 1000, 0); // 同期タイミングを戻す

    // WiFiを切断
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
}