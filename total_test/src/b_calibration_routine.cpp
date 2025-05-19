#include <HX711.h>
#include "globals.h"
#include "b_calibration_util.h"
#include "b_calibration_routine.h"

extern HX711 hx711;

void runCalibration() {
  Serial.println("🛠 [1단계] tare 시작: 사료통만 올려놓고 아무 키나 누르세요.");
  while (!Serial.available());
  Serial.read();

  hx711.tare();
  Serial.println("✅ tare 완료");

  Serial.println("🛠 [2단계] 기준추(예: 750g)를 올리고 'b'를 입력하세요.");
  while (!Serial.available());
  if (Serial.read() != 'b') return;

  delay(2000);  // 안정화 대기
  float raw = hx711.get_units(10);
  float weight = 750.0;  // 기준 추 무게 (단위: g)
  float factor = raw / weight;

  calibration_factor = factor;
  saveCalibrationFactor(factor);

  Serial.print("✅ 보정값 계산 완료: ");
  Serial.println(factor, 4);
}
