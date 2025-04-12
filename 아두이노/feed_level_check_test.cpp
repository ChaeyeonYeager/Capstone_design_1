// 센서가 잘 연결됐는지
// 센서가 측정값을 제대로 주는지
// 퍼센트 계산이 잘 되는지
// LED가 기준 퍼센트(20%)에서 잘 반응하는지
// 만 확인하는 코드

#include <Arduino.h>

#define TRIG1 A0
#define ECHO1 A1
#define TRIG2 A2
#define ECHO2 A3
#define LED_PIN 13

const float MAX_HEIGHT = 30.0;

void setup() {
  Serial.begin(9600);

  pinMode(TRIG1, OUTPUT); pinMode(ECHO1, INPUT);
  pinMode(TRIG2, OUTPUT); pinMode(ECHO2, INPUT);
  pinMode(LED_PIN, OUTPUT);
}

float measureDistance(int trig, int echo) {
  digitalWrite(trig, LOW); delayMicroseconds(2);
  digitalWrite(trig, HIGH); delayMicroseconds(10);
  digitalWrite(trig, LOW);
  long duration = pulseIn(echo, HIGH, 30000);
  return duration * 0.0343 / 2.0;
}

float getAverageDistance() {
  float d1 = measureDistance(TRIG1, ECHO1);
  float d2 = measureDistance(TRIG2, ECHO2);
  return (d1 + d2) / 2.0;
}

void checkFoodLevel() {
  float distance = getAverageDistance();
  float foodHeight = MAX_HEIGHT - distance;
  float foodPercent = (foodHeight / MAX_HEIGHT) * 100.0;

  Serial.print("사료 잔량: ");
  Serial.print(foodPercent);
  Serial.println("%");

  if (foodPercent <= 20.0) {
    digitalWrite(LED_PIN, HIGH);   // 잔량 부족 시 LED 켜짐
  } else {
    digitalWrite(LED_PIN, LOW);    // 잔량 충분 시 LED 꺼짐
  }
}

void loop() {
  checkFoodLevel();   // 계속 측정
  delay(1000);        // 1초마다 반복 측정
}
