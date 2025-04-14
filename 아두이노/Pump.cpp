#include <Arduino.h>
#include "Pump.h"
#include "FlowCheck.h"
#include "FlowCheck.cpp"

#define RELAY_PIN 8

FlowSensor flowSensor(2);  // 인터럽트 핀 2번

void PumpController::begin() {
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);  // 시작 시 펌프 ON (열림 상태)
  pumpState = true;

  flowSensor.begin();
}

void PumpController::update() {
  flowSensor.update(); // 업데이트 후
  flowSensor.targetWater(); // 목표 유량에 도달했는지 확인
  
  // 펌프가 열려있고 flowSensor.targetWater 함수의 반환 값이 true라면..
  if (pumpState && flowSensor.targetWater()) {
    digitalWrite(RELAY_PIN, LOW);  // 펌프 OFF (닫힘)
    pumpState = false;
  }
}

//펌프 state 업데이트 후 값 반환
bool PumpController::isPumpOn() {
  return pumpState; // false인 경우가 잠긴 것..
}
