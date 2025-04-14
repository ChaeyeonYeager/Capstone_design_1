#include <Arduino.h>
#include "FlowCheck.h"
#include "Pump.h"

FlowSensor flowSensor(2, 450); // 인터럽트 핀 2번 사용, 리터당 펄스는 450
PumpController pump; 
bool lastPumpState = true; // 펌프는 열려있는 상태로 시작

void setup() {
    Serial.begin(9600);
    flowSensor.begin();
}

// 유량이 계산되고 있는지 테스트 코드
void loop() {
    flowSensor.update(); // 1초마다 유량 갱신
    Serial.print("유량 (mL/s): "); // 흐르고 있는 유량 출력
    Serial.print(flowSensor.getFlowRate()); 

    Serial.print(" / 누적 유량 (mL): "); // 지금까지 흐른 누적 물 양 출력력
    Serial.println(flowSensor.getTotalVolume());

    pump.update();

    bool currentPumpState = pump.isPumpOn(); // 펌프 상태 값 받기기
    if (currentPumpState != lastPumpState) { // 상태가 달라질 때만 출력력
      if (currentPumpState) {
        Serial.println("펌프 열림");
      } else {
        Serial.println("펌프 닫힘 - 목표 유량 도달");
      }
      lastPumpState = currentPumpState;
    }

    delay(1000); // 1초마다 출력
}
