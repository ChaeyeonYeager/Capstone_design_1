#include <Arduino.h>

// L298N 모터 드라이버 핀 연결
const int IN1 = 8;
const int IN2 = 9;
const int ENA = 10;  // PWM 가능 핀

void setup() {
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENA, OUTPUT);

  Serial.begin(9600);
  Serial.println("워터펌프 모터 드라이버 제어 시작");
}

void loop() {
  // 펌프 ON
  Serial.println("펌프 ON");
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, 200);  // 속도 200 (0~255)

  delay(5000);  // 5초간 작동

  // 펌프 OFF
  Serial.println("펌프 OFF");
  analogWrite(ENA, 0);  // 전압 0 → 정지
  delay(5000);  // 5초간 정지
}
