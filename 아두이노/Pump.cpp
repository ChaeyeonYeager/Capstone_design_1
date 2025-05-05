#include <Arduino.h>
#include "FlowCheck.h"
#include "Pump.h"

#define RELAY_PIN 8

bool pumpState;

void initPump() {
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);  // 펌프 ON
  pumpState = true;

  initFlowSensor();               // 유량 센서 초기화 (핀/펄스 고정)
}

void pumpUpdate() {
  flowUpdate();                   // 유량 갱신

  if (pumpState && targetWater()) {
    digitalWrite(RELAY_PIN, LOW); // 펌프 OFF
    pumpState = false;
  }

  delay(100);
}

bool isPumpOn() {
  return pumpState;
}


//메인에서
// #include <Arduino.h>
// #include "Pump.h"

// void setup() {
//   Serial.begin(9600);
//   initPump();  // 함수 명 통일됨
// }

// void loop() {
//   pumpUpdate();
// }
