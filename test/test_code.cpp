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

    Serial.print("\n누적 유량 (mL): "); // 지금까지 흐른 누적 물 양 출력
    Serial.println(flowSensor.getTotalVolume());

    // floweSensor.target 호출 후 타겟 유량에 도달하였는지 확인 
    // 맞으면 실행
    if(flowSensor.targetWater()){
      pump.update();

      // 펌프 상태 업데이트 후 제대로 닫혔는지 확인
      bool currentPumpState = pump.isPumpOn(); // 펌프 상태 값 받아보기
      if (currentPumpState) {
        Serial.println("\n펌프가 열려있음");
      } 
      else { //성공!!
        Serial.println("\n펌프가 닫힘 - 목표 유량 도달");
        return;
      }
    }

    delay(1000); // 1초마다 출력
}
