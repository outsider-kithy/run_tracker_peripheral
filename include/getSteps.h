#pragma once
#include <Arduino.h>
#include <M5Unified.h>

// 加速度のしきい値（この値を超えたら1歩とカウント）
const float STEP_THRESHOLD = 0.9;   

// 状態変数
extern int steps;
extern bool stepActive;

int steps = 0;
bool stepActive = false;

void setupSteps() {

  if (!M5.Imu.isEnabled()) {
    M5.Imu.begin();
  }

  delay(500);
}

//歩数カウントをスタート
void startCountSteps() {
  M5.update();

  float accX, accY, accZ;
  M5.Imu.getAccel(&accX, &accY, &accZ);

  // 加速度の合成値（ベクトル長）
  float magnitude = sqrt(accX * accX + accY * accY + accZ * accZ);

  // しきい値を超えたら「1歩」
  if (magnitude > STEP_THRESHOLD && !stepActive) {
    stepActive = true;
    steps++;
  }

  // 一定値を下回ったら「次のステップ検出可能状態」に戻す
  if (magnitude < 0.5) {
    stepActive = false;
  }
  
  delay(200);
}

//歩数カウントをストップ
void stopCountSteps(){
	M5.Lcd.setCursor(0, 60);
	M5.Lcd.print(steps);
	M5.Lcd.println(" steps");
}