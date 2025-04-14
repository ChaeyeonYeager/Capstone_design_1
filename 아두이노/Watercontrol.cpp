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

volatile int flowPulseCount = 0;
unsigned long lastPulseTime = 0;

void initWaterSystem() {
  pinMode(pumpPin, OUTPUT);
  pinMode(flowSensorPin, INPUT_PULLUP);
  digitalWrite(pumpPin, LOW);

  attachInterrupt(digitalPinToInterrupt(flowSensorPin), countFlowPulse, RISING);

  Serial.print("목표 펄스 수 (100ml 기준): ");
  Serial.println(targetPulseCount);
}

void startWaterInjection() {
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
}

void waitForSoaking() {
  Serial.println("불림 대기 시작 (10분)");
  delay(600000);
  Serial.println("불림 완료");
}

void countFlowPulse() {
  flowPulseCount++;
}
