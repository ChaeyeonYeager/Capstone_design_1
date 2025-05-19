#include <Arduino.h>
#include "WaterControl.h"

// ===== 내부 상태 변수 =====
unsigned int pulseCount = 0;      // 유량 센서 펄스 수 (인터럽트로 증가)
static unsigned int lastState = HIGH;      // 이전 상태 기억(유량센서 풀업저항으로 설정해서 초기 상태 HIGH)
static unsigned long lastPulseTime = 0; // 마지막 펄스 카운트 시간

bool isSoaking = false;               // 불림 완료 여부 플래그
bool isProcessDone = false;           // 전체 프로세스 완료 여부 플래그

// ✅ 시스템 초기화
void initWaterSystem() {
  Serial.begin(9600);
  pinMode(relayPin, OUTPUT);  // 펌프 제어 핀
  pinMode(flowSensorPin, INPUT_PULLUP); // 유량 센서 풀업 저항 설정
  digitalWrite(relayPin, LOW);  // 펌프 OFF
  Serial.println("시스템 초기화 완료");
}

// ✅ 상태 기반 펄스 카운터 (loop에서 호출)
void updatePulseCount() {
  int currentState = digitalRead(flowSensorPin);
  if (currentState == LOW && lastState == HIGH) {
    pulseCount++;
  }
  lastState = currentState;
}

// ✅ 워터 프로세스 실행 (1회만)
void runWaterProcess() {
  if (isProcessDone) return;

  pulseCount = 0;
  Serial.println("물 투입 시작");
  digitalWrite(relayPin, HIGH);  // 펌프 ON

  while (pulseCount < targetPulseCount) {
    updatePulseCount();  // 상태 변화 기반 펄스 카운트

    if (millis() - lastPulseTime > 1000) {
      Serial.print("현재 펄스 수: ");
      Serial.println(pulseCount);
      lastPulseTime = millis();
    }
  }

  digitalWrite(relayPin, LOW);  // 펌프 OFF
  Serial.println("물 투입 완료");

  Serial.println("불림 대기 시작 (30분)");
  delay(1800000);  // 30분 대기
  Serial.println("불림 완료");

  isSoaking = true;
  isProcessDone = true;
}

// ✅ 외부에서 불림 완료 여부 확인 함수
bool isSoakingDone() {
  return isSoaking;
}


// ===== 메인 함수 예시 =====


// void setup() {
//   initWaterSystem();
// }

// void loop() {
//   runWaterProcess();

//   // 이후 추가 동작 가능
//   // 예: if (isSoakingDone()) { ... }
// }
