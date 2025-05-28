// src/e_feed_grinder.cpp

#include <Arduino.h>
#include "e_feed_grinder.h"
#include "globals.h"

void initmotorGrinder() {
  pinMode(RPWM, OUTPUT);
  pinMode(LPWM, OUTPUT);
  pinMode(R_EN, OUTPUT);
  pinMode(L_EN, OUTPUT);

  digitalWrite(R_EN, HIGH);
  digitalWrite(L_EN, HIGH);  
}

// ✅ 모터 동작 순서 함수 (속도 100으로 1분 30초 회전)
void motorGrinder() {
  Serial.println(F("🔄 분쇄 시작: 속도 100, 1분 30초 회전"));
  rotateMotor(255);      // 속도 100으로 회전
  delay(90000);          // 90초(1분 30초) 유지

  Serial.println(F("🛑 분쇄 완료, 모터 정지"));
  delay(500);            // 짧게 대기


  isGrinding = true;     // 분쇄 완료 플래그
}

// ✅ 정방향 회전 함수 (속도 지정)
void rotateMotor(int speed) {
  analogWrite(RPWM, speed);
  analogWrite(LPWM, 0);  // 역방향은 항상 0
}


bool isGrindingDone() {
  return isGrinding;
}
