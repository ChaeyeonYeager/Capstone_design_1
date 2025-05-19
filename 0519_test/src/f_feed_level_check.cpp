#include "f_feed_level_check.h"
#include <Arduino.h>
#include "globals.h"


#define TRIG1 A0
#define ECHO1 A1
#define TRIG2 A2
#define ECHO2 A3
#define LED_PIN 13

const float SIMULATED_EMPTY = 40.0;
const float SIMULATED_FULL = 10.0;
const float DISTANCE_MIN = 2.0;
const float DISTANCE_MAX = 400.0;

void initFeedingSystem() {
  pinMode(TRIG1, OUTPUT); pinMode(ECHO1, INPUT);
  pinMode(TRIG2, OUTPUT); pinMode(ECHO2, INPUT);
  pinMode(LED_PIN, OUTPUT);
}

float measureDistance(int trig, int echo, const char* label) {
  digitalWrite(trig, LOW); delayMicroseconds(2);
  digitalWrite(trig, HIGH); delayMicroseconds(10);
  digitalWrite(trig, LOW);

  long duration = pulseIn(echo, HIGH, 30000);
  float distance = duration * 0.0343 / 2.0;

  Serial.print(label);
  Serial.print(" duration: ");
  Serial.print(duration);
  Serial.print(" µs → distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  if (duration == 0 || distance < DISTANCE_MIN || distance > DISTANCE_MAX) {
    return 999.0;
  }
  return distance;
}

float getAverageDistance() {
  float d1 = measureDistance(TRIG1, ECHO1, "센서1");
  delay(400);
  float d2 = measureDistance(TRIG2, ECHO2, "센서2");

  if (d1 > 500 && d2 > 500) {
    Serial.println("⚠ 두 센서 모두 실패");
    return 999.0;
  } else if (d1 > 500) {
    Serial.println("⚠ 센서1 실패 → 센서2 사용");
    return d2;
  } else if (d2 > 500) {
    Serial.println("⚠ 센서2 실패 → 센서1 사용");
    return d1;
  }

  Serial.println("✅ 두 센서 정상 → 평균 사용");
  return (d1 + d2) / 2.0;
}

void checkFoodLevel() {
  float distance = getAverageDistance();

  if (distance > 500.0) {
    Serial.println("❗ 거리 체크 실패 (최종)");
    digitalWrite(LED_PIN, HIGH);
    return;
  }

  float percent = 100.0 * (SIMULATED_EMPTY - distance) / (SIMULATED_EMPTY - SIMULATED_FULL);
  percent = constrain(percent, 0.0, 100.0);

  Serial.print("📏 평균 거리: ");
  Serial.print(distance);
  Serial.print(" cm → 잔량: ");
  Serial.print(percent);
  Serial.println(" %");

  digitalWrite(LED_PIN, (percent <= 20.0) ? HIGH : LOW);
}