#include <Arduino.h>

#define TRIG1 A0
#define ECHO1 A1
#define TRIG2 A2
#define ECHO2 A3
#define LED_PIN 13

const float CYLINDER_HEIGHT = 9.5; // 원기둥 높이
const float CONE_HEIGHT = 4.5;     // 삼각뿔 높이
const float TOTAL_HEIGHT = CYLINDER_HEIGHT + CONE_HEIGHT;

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
  long duration = pulseIn(echo, HIGH, 30000); // 30ms timeout
  return duration * 0.0343 / 2.0;
}

float getAverageDistance() {
  float d1 = measureDistance(TRIG1, ECHO1);
  float d2 = measureDistance(TRIG2, ECHO2);
  return (d1 + d2) / 2.0;
}

void checkFoodLevel() {
  float distance = getAverageDistance();
  float foodHeight = TOTAL_HEIGHT - distance;

  float percent = 0.0;

  if (foodHeight <= 0) {
    percent = 0.0;
  } else if (foodHeight <= CONE_HEIGHT) {
    // 삼각뿔 부분은 부피가 높이에 비례하지 않고 세제곱에 비례
    float ratio = foodHeight / CONE_HEIGHT;
    percent = (1.0 / 4.0) * pow(ratio, 3) * 100.0;
  } else {
    // 원기둥 + 삼각뿔 부피
    float coneVolume = 1.0 / 4.0 * 100.0; // 삼각뿔 최대 퍼센트 (25%)
    float cylinderRatio = (foodHeight - CONE_HEIGHT) / CYLINDER_HEIGHT;
    percent = coneVolume + cylinderRatio * (100.0 - coneVolume);
  }

  Serial.print("사료 잔량: ");
  Serial.print(percent);
  Serial.println("%");

  if (percent <= 20.0) {
    digitalWrite(LED_PIN, HIGH);
  } else {
    digitalWrite(LED_PIN, LOW);
  }
}

void loop() {
  checkFoodLevel();
  delay(1000);
}
