#include <Arduino.h>
#include "FlowCheck.h"
#include "Pump.h"
#define RELAY_PIN 8

bool pumpState;

void setup() {
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);  // 펌프 ON (열림 상태)
  pumpState = true;

  FlowSensor(2, 450); // 초기화 함수 (핀 번호, 펄스 당 리터 수)
  begin();            // 센서 인터럽트 초기화
}

void pumpUpdate() {
  flowUpdate(); // 유량 갱신

  if (pumpState && targetWater()) {
    digitalWrite(RELAY_PIN, LOW);  // 펌프 OFF (닫힘)
    pumpState = false;
  }

  delay(100); // 너무 자주 체크하지 않도록 약간의 딜레이
}

bool isPumpOn() {
  return pumpState;
}
