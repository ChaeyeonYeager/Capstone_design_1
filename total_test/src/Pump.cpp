#include <Arduino.h>
#include "FlowCheck.h"  // 유량 측정 관련 함수
#include "Pump.h"

#define RELAY_PIN 8      // 릴레이 제어 핀 (펌프 연결)

bool pumpState;          // 펌프 작동 상태 플래그

// ✅ 펌프 초기화 함수
void initPump() {
  pinMode(RELAY_PIN, OUTPUT);      // 릴레이 핀 출력 모드로 설정
  digitalWrite(RELAY_PIN, HIGH);   // 펌프 ON (릴레이 HIGH)
  pumpState = true;

  initFlowSensor(2, 450);  // 핀 번호, 펄스/리터
  beginFlowSensor();
}

// ✅ 주기적으로 호출되어 유량 체크 및 펌프 OFF 조건 판단
void pumpUpdate() {
  flowUpdate();                    // 1초마다 유량 측정 갱신

  // 목표 유량에 도달하면 펌프 OFF
  if (pumpState && targetWater()) {
    digitalWrite(RELAY_PIN, LOW);  // 릴레이 OFF → 펌프 OFF
    pumpState = false;
  }

  delay(100);  // 너무 자주 체크하지 않도록 딜레이
}

// ✅ 펌프가 현재 작동 중인지 여부 반환
bool isPumpOn() {
  return pumpState;
}
