#pragma once
#include <M5Unified.h>
#include <Preferences.h>

Preferences preferences;

// 初期値
float STEP_THRESHOLD = 0.35f;

int steps = 0;
bool stepActive = false;

// 歩行ピークからしきい値を決める倍率
const float CALIBRATION_RATIO = 0.5f;
// キャリブレーション時間
const uint32_t CALIBRATION_TIME = 10000;

// キャリブレーション
void calibrateStepThreshold() {
    Serial.println("Start step calibration");
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(0, 20);
	M5.Lcd.setTextColor(WHITE);
    M5.Lcd.println("Calibration");
    M5.Lcd.println("");
    M5.Lcd.println("Walk for 10 sec.");
    delay(100);

    uint32_t startTime = millis();
    float maxDynamicAccel = 0.0f;
    while (millis() - startTime < CALIBRATION_TIME) {
        M5.update();
        float accX, accY, accZ;
        if (M5.Imu.getAccel(&accX, &accY, &accZ)) {

            float magnitude = sqrt(accX * accX + accY * accY + accZ * accZ);
            // 重力加速度1Gを除外
            float dynamicAccel = fabs(magnitude - 1.0f);

            if (dynamicAccel > maxDynamicAccel) {
                maxDynamicAccel = dynamicAccel;
            }
            Serial.printf("dynamicAccel = %.3f\n", dynamicAccel);
        }

        delay(20);
    }

    // 最大加速度からしきい値を計算
    STEP_THRESHOLD = maxDynamicAccel * CALIBRATION_RATIO;

    // 異常値対策
    if (STEP_THRESHOLD < 0.15f) {
        STEP_THRESHOLD = 0.15f;
    }

    if (STEP_THRESHOLD > 1.0f) {
        STEP_THRESHOLD = 1.0f;
    }

    Serial.printf("Step threshold = %.3f\n", STEP_THRESHOLD);

    // NVSへ保存
    preferences.begin("step", false);
    preferences.putFloat("threshold", STEP_THRESHOLD);
    preferences.putBool("calibrated", true);
    preferences.end();

    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(0, 20);
	M5.Lcd.setTextColor(WHITE);
    M5.Lcd.println("Calibration OK");
    M5.Lcd.printf("Threshold: %.3f", STEP_THRESHOLD);
    delay(20);
	M5.Lcd.fillScreen(BLACK);
}


// 初期化
void setupSteps() {

    if (!M5.Imu.isEnabled()) {
        M5.Imu.begin();
    }
    delay(50);

    // 保存済みのキャリブレーション値を確認
    preferences.begin("step", true);

    bool calibrated = preferences.getBool("calibrated", false);

    if (calibrated) {
        STEP_THRESHOLD = preferences.getFloat("threshold", 0.35f);
        Serial.printf("Loaded STEP_THRESHOLD: %.3f\n", STEP_THRESHOLD);
        preferences.end();
    } else {
		STEP_THRESHOLD = 0.35f;
        preferences.end();
    }
}

// 歩数カウント開始
void updateSteps() {

    float accX, accY, accZ;

    if (!M5.Imu.getAccel(&accX, &accY, &accZ)) {
        return;
    }

    // 加速度ベクトル
    float magnitude = sqrt(accX * accX + accY * accY + accZ * accZ);

    // 重力成分を除外
    float dynamicAccel = fabs(magnitude - 1.0f);

    // 歩行ピーク検出
    if (dynamicAccel > STEP_THRESHOLD && !stepActive) {
        stepActive = true;
        steps++;
        // Serial.printf(
        //     "STEP %d accel=%.3f\n",
        //     steps,
        //     dynamicAccel
        // );
    }

    // しきい値より十分小さくなったら次の歩行を検出可能にする
    if (dynamicAccel < STEP_THRESHOLD * 0.5f) {
        stepActive = false;
    }

    delay(20);
}

// 歩数カウント開始
void startCountSteps(){
	steps = 0;
}

// 歩数カウント停止
void stopCountSteps() {
    M5.Lcd.setCursor(0, 60);
	M5.Lcd.setTextColor(WHITE);
    M5.Lcd.print(steps);
    M5.Lcd.println(" steps,");
}

