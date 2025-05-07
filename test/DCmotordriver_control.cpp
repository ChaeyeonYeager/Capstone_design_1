#include <Arduino.h>

// 펌프는 100mL를 주입하는 것을 목표로 합니다.
// 고정된 펄스(98회)를 기준으로 하여 유량 센서의 펄스를 카운트합니다.
// 펌프가 작동하면 유량 센서에서 펄스가 발생합니다.
// 이 펄스는 인터럽트로 카운트됩니다.
// 펌프가 작동하는 동안 유량 센서의 펄스 수를 체크하여 목표 펄스 수에 도달하면 펌프를 정지합니다.
// 펌프는 5초간 정지 후 다시 작동할 수 있도록 대기합니다.

// 펌프 제어 핀
const int IN1 = 8;
const int IN2 = 9;
const int ENA = 10;

// 유량 센서 핀
const int flowSensorPin = 2;
volatile int pulseCount = 0;

const int targetPulse = 98; // 100mL 목표 펄스 수 (YM-401A 기준)

bool pumpRunning = false;

void setup() {
  // 핀 설정
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(flowSensorPin, INPUT_PULLUP);

  // 인터럽트 등록
  attachInterrupt(digitalPinToInterrupt(flowSensorPin), countPulse, RISING);

  // 시리얼 시작
  Serial.begin(9600);
  Serial.println("=== 100mL 정량 주입 시스템 시작 ===");
}

void loop() {
  // 1회만 실행 (펌프 동작)
  if (!pumpRunning) {
    pulseCount = 0;
    pumpRunning = true;

    // 펌프 ON
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    analogWrite(ENA, 191); // 9V 수준
    Serial.println("[펌프 ON] 100mL 목표로 주입 시작...");
  }

  // 실시간 펄스 체크
  if (pumpRunning && pulseCount >= targetPulse) {
    // 펌프 OFF
    analogWrite(ENA, 0);
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    pumpRunning = false;

    Serial.println("[펌프 OFF] 목표 100mL 도달!");
    Serial.print("최종 펄스 수: ");
    Serial.println(pulseCount);
    float ml = pulseCount * (100.0 / targetPulse); // 100mL 기준 환산
    Serial.print("계산된 주입량 (mL): ");
    Serial.println(ml, 1);

    Serial.println("5초간 대기 후 재시작 가능...\n");
    delay(5000);
  }
}

void countPulse() {
  pulseCount++;
}
