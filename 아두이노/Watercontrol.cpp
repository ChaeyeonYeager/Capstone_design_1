#include <Arduino.h>
#include "WaterControl.h"

// ===== 내부 상태 변수 =====
volatile int flowPulseCount = 0;      // 유량 센서 펄스 수 (인터럽트로 증가)
unsigned long lastPulseTime = 0;      // 마지막으로 펄스를 출력한 시간

bool isSoaking = false;               // 불림 완료 여부 플래그
bool isProcessDone = false;           // 전체 프로세스 완료 여부 플래그

// ✅ 시스템 초기화: 핀 모드 설정 및 인터럽트 연결
void initWaterSystem() {
  pinMode(pumpPin, OUTPUT);                   // 펌프 제어 핀
  pinMode(flowSensorPin, INPUT_PULLUP);       // 유량 센서 입력 핀
  digitalWrite(pumpPin, LOW);                 // 초기 상태: 펌프 OFF

  attachInterrupt(digitalPinToInterrupt(flowSensorPin), countFlowPulse, RISING); // 인터럽트 설정

  Serial.print("목표 펄스 수 (100ml 기준): ");
  Serial.println(targetPulseCount);
}

// ✅ 유량 센서 인터럽트 함수: 펄스 1회 감지 시 호출됨
void countFlowPulse() {
  flowPulseCount++;
}

// ✅ 전체 워터 프로세스 실행 함수 (1회만 실행됨)
void runWaterProcess() {
  if (isProcessDone) return;  // 이미 완료된 경우 중복 실행 방지

  // 1. 물 투입 시작
  flowPulseCount = 0;
  Serial.println("물 투입 시작");
  digitalWrite(pumpPin, HIGH);  // 펌프 ON

  // 목표 펄스 수 도달 시까지 대기
  while (flowPulseCount < targetPulseCount) {
    if (millis() - lastPulseTime > 1000) {
      Serial.print("현재 펄스 수: ");
      Serial.println(flowPulseCount);
      lastPulseTime = millis();
    }
  }

  digitalWrite(pumpPin, LOW);  // 펌프 OFF
  Serial.println("물 투입 완료");

  // 2. 사료 불림 대기 시간 (30분)
  Serial.println("불림 대기 시작 (30분)");
  delay(1800000);  // ✅ 30분 대기 (1800000 ms)
  Serial.println("불림 완료");

  isSoaking = true;
  isProcessDone = true;
}

// ✅ 외부에서 불림 완료 여부 확인 함수
bool isSoakingDone() {
  return isSoaking;
}
