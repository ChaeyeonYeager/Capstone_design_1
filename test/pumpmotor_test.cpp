#include <Arduino.h>
const int relayPin = 7;  // S 핀을 아두이노 D7에 연결

void setup() {
  pinMode(relayPin, OUTPUT);
  Serial.begin(9600);
  Serial.println("워터펌프 릴레이 제어 시작");
}

void loop() {
  Serial.println("펌프 ON");
  digitalWrite(relayPin, LOW);  // 릴레이 ON (보통 LOW에서 작동)
  delay(5000);  // 5초 동안 펌프 작동

  Serial.println("펌프 OFF");
  digitalWrite(relayPin, HIGH);  // 릴레이 OFF
  delay(5000);  // 5초 정지
}
