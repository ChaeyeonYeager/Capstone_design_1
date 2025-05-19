#include <Arduino.h>

const int flowSensorPin = 2;
const int relayPin = 3;

unsigned int pulseCount = 0;
int lastState = HIGH;  // 이전 상태 기억

const unsigned int targetPulse = 515;  // 목표 펄스 수 (100mL)

void setup() {
  Serial.begin(9600);
  pinMode(flowSensorPin, INPUT_PULLUP);
  pinMode(relayPin, OUTPUT);

  digitalWrite(relayPin, HIGH);  // 펌프 ON
  Serial.println("펌프 시작: 목표 100mL");
}

void loop() {
  int currentState = digitalRead(flowSensorPin);

  if (currentState == LOW && lastState == HIGH) {
    pulseCount++;
    Serial.print("PulseCount    ");
    Serial.println(pulseCount);

    if (pulseCount >= targetPulse) {
      digitalWrite(relayPin, LOW);  // 펌프 OFF
      Serial.println("목표 도달: 100mL 주입 완료");
      while (true) {
        delay(1000);  // 멈춤
      }
    }
  }

  lastState = currentState;

  delay(1);  // 너무 빠른 루프 방지
}
