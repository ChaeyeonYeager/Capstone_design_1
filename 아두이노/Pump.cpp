#include <Arduino.h>
#include "Pump.h"
#include "FlowCheck.h"

#define RELAY_PIN 8
#define TARGET_VOLUME_ML 200  // 목표 유량 ==> 이건 앞으로 수정해나가야..

FlowSensor flowSensor(2);  // 인터럽트 핀 2번

void PumpController::begin() {
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);  // 시작 시 펌프 ON (열림 상태)
  pumpState = true;

  flowSensor.begin();
}

void PumpController::update() {
  flowSensor.update();
  
  // 펌프가 열려있고 토탈 유량이 타겟보다 많거나 같으면 닫기
  if (pumpState && flowSensor.getTotalVolume() >= TARGET_VOLUME_ML) {
    digitalWrite(RELAY_PIN, LOW);  // 펌프 OFF (닫힘)
    pumpState = false;
  }
}

//펌프 state 업데이트 후 값 반환환
bool PumpController::isPumpOn() {
  return pumpState;
}
