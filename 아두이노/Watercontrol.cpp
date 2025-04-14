#include "WaterControl.h"

/*
    동작 설명
    1. DC 펌프를 통해 물을 투입(DC모터 작동 (ON) → 물이 유량계를 지나며 사료통으로 이동)
    2. 유량 센서로 물의 양을 측정유량계 펄스 측정 → 목표 펄스(=목표 유량) 도달 시 DC모터 작동 (OFF))
    3. 물 투입이 완료되면 체크 밸브를 닫음(DC모터 작동 (OFF) → 물이 유량계를 지나지 않음)
*/

/*
    주의 사항
    1. DC 펌프와 유량 체크는 전자식으로 작동
    2. 유량 센서는 펄스 신호를 발생시키며, 이 신호를 통해 물의 양을 측정합니다.
    3. 유량 센서의 펄스 수를 카운트하여 목표 물 양(100ml)에 도달했는지 확인합니다.
    4. 물 투입이 완료되면 체크 밸브(수동 부품)를 닫고, DC 펌프를 끕니다.
    5. 물 투입이 완료된 후에는 사료를 불리기 위해 대기합니다.
    6. 사료 불림 대기 시간은 10분으로 설정되어 있습니다.
*/  

#include "WaterControl.h"

volatile int flowPulseCount = 0;  // 유량 센서에서 발생한 펄스 수
unsigned long lastPulseTime = 0;  // 마지막 펄스 시간

void initWaterSystem() {
  pinMode(pumpPin, OUTPUT);
  pinMode(flowSensorPin, INPUT_PULLUP);
  digitalWrite(pumpPin, LOW);

  attachInterrupt(digitalPinToInterrupt(flowSensorPin), countFlowPulse, RISING);  // 유량 센서의 펄스 신호를 감지하여 countFlowPulse 함수를 호출

  Serial.print("목표 펄스 수 (100ml 기준): ");  // 목표 펄스 수 출력
  Serial.println(targetPulseCount); // 예: 100ml 기준 펄스 수
}

void startWaterInjection() {
  flowPulseCount = 0; // 펄스 카운트 초기화
  Serial.println("물 투입 시작"); // 물 투입 시작 메시지 출력

  digitalWrite(pumpPin, HIGH);  // DC 펌프 작동 (ON)

  while (flowPulseCount < targetPulseCount) { // 목표 펄스 수에 도달할 때까지 반복
    if (millis() - lastPulseTime > 1000) {  // 1초마다 펄스 수 출력
      Serial.print("현재 펄스 수: ");
      Serial.println(flowPulseCount); // 현재 펄스 수 출력
      lastPulseTime = millis();
    }
  }

  digitalWrite(pumpPin, LOW); // DC 펌프 정지 (OFF)
  Serial.println("물 투입 완료");
}

void waitForSoaking() {
  Serial.println("불림 대기 시작 (10분)");
  delay(600000);  // 10분 대기 (600000ms)
  Serial.println("불림 완료");

  return true; // 불림 완료 후 true 반환
}

void countFlowPulse() {
  flowPulseCount++; // 펄스 카운트 증가
}

