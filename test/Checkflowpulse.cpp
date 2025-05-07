#include <Arduino.h>

// 오직 DC 모터 동작과 이를 통하여 유랑 센서의 100ml 물 펄스를 체크하기 위한 코드

/*
  DC 펌프를 10초 동안 동작시켜 물 100ml를 유량 센서를 통해 주입하고, 
  센서에서 발생하는 펄스를 카운트하여 유량을 측정
  측정된 펄스 수를 바탕으로 이후 DC 모터 제어 로직에 활용
*/


// 핀 설정
const int pumpPin = 7;          // DC 펌프 제어 핀(릴레리 모듈 S->D7)
const int flowSensorPin = 6;    // 유량 센서 핀
volatile int flowPulseCount = 0;  // 유량 센서 펄스 카운트

unsigned long startTime;
const unsigned long pumpDuration = 10000; // 펌프 작동 시간 (10초)

void setup() {
  Serial.begin(9600);

  pinMode(pumpPin, OUTPUT); // DC 펌프 핀 설정
  pinMode(flowSensorPin, INPUT_PULLUP); // 유량 센서 핀 설정 (풀업 저항 사용)

  digitalWrite(pumpPin, LOW); // 시작 시 펌프 OFF

  attachInterrupt(digitalPinToInterrupt(flowSensorPin), countFlowPulse, RISING);  // 유량 센서 인터럽트 설정

  Serial.println("실험 시작: DC 펌프 ON, 100ml가 채워질 때까지 펄스를 측정합니다.");

  // 펌프 ON
  digitalWrite(pumpPin, HIGH);
  startTime = millis();
}

void loop() {
  // 실험 종료 조건: 10초 후 펌프 자동 정지
  if (millis() - startTime >= pumpDuration) {
    digitalWrite(pumpPin, LOW);
    Serial.println("펌프 작동 시간 종료, 자동 OFF");
    Serial.print("측정된 펄스 수: ");
    Serial.println(flowPulseCount);
    while (true); // 종료 후 멈춤
  }

  // 실시간 펄스 수 출력
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 1000) {
    Serial.print("현재 펄스 수: ");
    Serial.println(flowPulseCount);
    lastPrint = millis();
  }
}

void countFlowPulse() {
  flowPulseCount++;
}



/*
  mlPerPulse = 100.0 / 측정된 펄스 수;

  int targetPulseCount = targetML / mlPerPulse;

*/