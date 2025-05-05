// 이 파일 삭제해주세요
// #include <Arduino.h>
// #include "FlowCheck.cpp"
// #include "Pump.cpp"

// bool lastPumpState = true; // 펌프는 열려있는 상태로 시작

// void setup() {
//     Serial.begin(9600);

//     FlowSensor(2, 450); // 핀 번호, 리터당 펄스 수 설정
//     begin();        
//     setup();        
// }

// void loop() {
//     flowUpdate(); // 유량 업데이트

//     Serial.print("유량 (mL/s): ");
//     Serial.print(getFlowRate());

//     Serial.print("\n누적 유량 (mL): ");
//     Serial.println(getTotalVolume());

//     if (targetWater()) { // 목표 유량 달성 했을시
//         pumpUpdate(); // 펌프 상태 체크 및 닫기

//         if (isPumpOn()) {
//             Serial.println("\n펌프가 열려있음, 오류!! 펌프 왜 안 닫혔어");
//         } else {
//             Serial.println("\n펌프가 닫힘 - 성공~~");
//             return;
//         }
//     }

//     delay(1000);
// }
