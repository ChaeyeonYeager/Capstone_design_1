#include <Arduino.h>
#include "WaterControl.h"

volatile int flowPulseCount = 0;
unsigned long lastPulseTime = 0;

bool isSoaking = false;
bool isProcessDone = false;

void initWaterSystem() {
  pinMode(pumpPin, OUTPUT);
  pinMode(flowSensorPin, INPUT_PULLUP);
  digitalWrite(pumpPin, LOW);

  attachInterrupt(digitalPinToInterrupt(flowSensorPin), countFlowPulse, RISING);

  Serial.print("목표 펄스 수 (100ml 기준): ");
  Serial.println(targetPulseCount);
}

void countFlowPulse() {
  flowPulseCount++;
}

// 통합된 워터 프로세스 실행 함수
void runWaterProcess() {
  if (isProcessDone) return; // 이미 완료되었으면 무시

  // 1. 물 투입
  flowPulseCount = 0;
  Serial.println("물 투입 시작");
  digitalWrite(pumpPin, HIGH);

  while (flowPulseCount < targetPulseCount) {
    if (millis() - lastPulseTime > 1000) {
      Serial.print("현재 펄스 수: ");
      Serial.println(flowPulseCount);
      lastPulseTime = millis();
    }
  }

  digitalWrite(pumpPin, LOW);
  Serial.println("물 투입 완료");

  // 2. 불림 대기
  Serial.println("불림 대기 시작 (10분)");
  delay(600000);  // 10분
  Serial.println("불림 완료");

  isSoaking = true;
  isProcessDone = true;
}

bool isSoakingDone() {
  return isSoaking;
}


// 메인에서 

// #include <Arduino.h>
// #include "WaterControl.h"

// void setup() {
//   Serial.begin(9600);
//   initWaterSystem();     // 핀/인터럽트 초기화
// }

// void loop() {
//   runWaterProcess();     // 물 투입 + 불림까지 자동 처리
// }
