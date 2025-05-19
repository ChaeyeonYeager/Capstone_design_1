// b_feeder.cpp
#include "b_feeder.h"
#include <Servo.h>
#include <EEPROM.h>
#include "globals.h"  // extern HX711 hx711_calc

// 핀 정의
#define DT_PIN      2
#define SCK_PIN     3
#define SERVO_PIN   9

// EEPROM 주소 (컨테이너 무게 저장용)
#define EEPROM_ADDR_CONTAINER  0

static Servo feederServo;
static float containerWeight = 0.0;

void initFeeder() {
  // 1) 로드셀 & 서보 초기화
  hx711_calc.begin(DT_PIN, SCK_PIN);
  feederServo.attach(SERVO_PIN);

  // 2) EEPROM에서 컨테이너 무게 불러오기
  EEPROM.get(EEPROM_ADDR_CONTAINER, containerWeight);
  Serial.print(F("Loaded container weight: "));
  Serial.print(containerWeight, 2);
  Serial.println(F(" g"));
}

void calibrateFeeder() {
  // 1) 아무것도 올리지 않은 상태에서 0점 보정
  Serial.println(F("[Calibration] 1) 0점 보정 - 아무것도 올리지 마세요"));
  hx711_calc.tare();
  delay(2000);

  // 2) 사료통만 올리고 사용자 키 입력 대기
  Serial.println(F("[Calibration] 2) 사료통 올리고 아무 키 입력"));
  while (!Serial.available());
  Serial.read();

  // 측정값 저장 및 EEPROM 기록
  containerWeight = hx711_calc.get_units(10);
  EEPROM.put(EEPROM_ADDR_CONTAINER, containerWeight);

  Serial.print(F("Saved container weight: "));
  Serial.print(containerWeight, 2);
  Serial.println(F(" g"));
}

void feedFoodProcess(float targetGrams) {
  Serial.print(F("▶ 사료 투입 목표: "));
  Serial.print(targetGrams, 2);
  Serial.println(F(" g"));

  // 1) 서보 열기
  feederServo.write(90);
  delay(500);

  // 2) 초기 사료량 확인 (전체 무게 - 컨테이너 무게)
  float startMass   = hx711_calc.get_units(10) - containerWeight;
  float currentMass = startMass;

  // 3) 목표량에 도달할 때까지 반복 측정
  while (currentMass < startMass + targetGrams) {
    currentMass = hx711_calc.get_units(10) - containerWeight;
    Serial.print(F("  현재 사료량: "));
    Serial.print(currentMass, 2);
    Serial.println(F(" g"));
    delay(500);
  }

  // 4) 서보 닫기
  feederServo.write(0);
  Serial.println(F("✅ 사료 투입 완료"));
}
