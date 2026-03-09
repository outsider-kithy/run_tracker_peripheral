#pragma once
#include <Arduino.h>
#include <M5Unified.h>
#include "getTime.h"

extern int steps;
extern bool stepActive;

// 加速度のしきい値（この値を超えたら1歩とカウント）
const float STEP_THRESHOLD = 0.12;  
// 1歩あたりの距離（m）※目安：歩幅約0.7m
const float STEP_LENGTH = 0.7;     

int steps = 0;
bool stepActive = false;

void setupSteps() {

  if (!M5.Imu.isEnabled()) {
    M5.Imu.begin();
  }

  delay(500);
}

void updateSteps() {
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
  if (magnitude < 0.9) {
    stepActive = false;
  }
  
  delay(200);

   if(!isTracking){
    if (M5.BtnA.wasReleased()) {
      delay(50);
   } else {
    if(M5.BtnA.wasPressed()){
      delay(50);
      steps *= 10;
    }
   }
  }
}
