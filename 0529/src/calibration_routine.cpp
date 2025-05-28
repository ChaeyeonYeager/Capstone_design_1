#include "calibration_routine.h"

// 전역 변수 정의 (여기서만!)
HX711 scale;
float calibration_factor = 1.0f;
float containerWeight     = 501.0f;

void calibrateZero() {
  Serial.println(F("🛠 Zero calibration (tare) in progress... press c to continue"));

  // 사용자가 'c'를 입력할 때까지 대기
  while (!Serial.available() || Serial.peek() != 'c') {
    delay(100);
  }
  Serial.read();  // 'c' 제거

  Serial.println(F("🛠 Performing tare..."));
  scale.tare();
  Serial.println(F("✅ Zero calibration done."));
}

void calibrateContainer() {
  Serial.println(F("🔧 Place container on scale. Press d to continue"));

  // 사용자가 'd'를 입력할 때까지 대기
  while (!Serial.available() || Serial.peek() != 'd') {
    delay(100);
  }
  Serial.read();  // 'd' 제거

  Serial.println(F("📏 Calibrating container..."));
  delay(2000);

  const int SAMPLES = 10;
  float sumRaw = 0.0f;
  for (int i = 0; i < SAMPLES; i++) {
    sumRaw += scale.get_units(3);
    delay(100);
  }
  float avgRaw = sumRaw / SAMPLES;
  calibration_factor = avgRaw / containerWeight;

  Serial.print(F("✅ Container calibration done. New scale factor: "));
  Serial.println(calibration_factor, 4);
}
