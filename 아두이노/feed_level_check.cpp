#include "feed_level_check.h"
#include <Arduino.h>

#define TRIG1 A0
#define ECHO1 A1
#define TRIG2 A2
#define ECHO2 A3
#define LED_PIN 13

const float MAX_HEIGHT = 30.0;
const unsigned long CHECK_DELAY = 1800000;  // 30 minutes

unsigned long feedingStartTime = 0;
bool feeding_done = false;
bool measuring_started = false;

void setupFeedingSystem() {
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
    digitalWrite(LED_PIN, HIGH);
  } else {
    digitalWrite(LED_PIN, LOW);
  }
}

bool isFoodInputDone() {
  // 이 부분은 사료가 투입되었는지 외부 조건에 따라 변경
  // 예시: 모터 동작 완료, 센서 상태 등
  return true; // 임시로 항상 true 반환
}

void handleFeedingLogic() {
  if (isFeedingDone() && !measuring_started) {
    feeding_done = true;
  }

  if (feeding_done && !measuring_started) {
    feedingStartTime = millis();
    measuring_started = true;
    Serial.println("✅ 배식 완료됨 → 30분 후 측정 예약");
  }

  if (measuring_started && millis() - feedingStartTime >= CHECK_DELAY) {
    Serial.println("⏱️ 30분 경과 → 사료 잔량 측정 중...");
    checkFoodLevel();
    feeding_done = false;
    measuring_started = false;
  }
} 


// 메인 제어 cpp 파일에서는

// void setup() {
//   Serial.begin(9600);
//   setupFeedingSystem();  // 센서 핀 세팅 등 초기 설정
// }

// void loop() {
//   handleFeedingLogic();  // 급식 후 30분 뒤 자동으로 잔량 측정 및 LED 제어
// }

// 이런식으로 호출
