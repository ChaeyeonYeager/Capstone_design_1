#include <Arduino.h>
#include <feeding_calc.h>

float calculatePortionGrams(int feedingCount, String feedTimes[], float dogWeight, float activeLvl, float calPerKg) {
  RER = 70 * pow(dogWeight, 0.75);               // 기초 에너지
  DER = RER * activeLvl;                         // 활동 반영 하루 에너지

  float grams = ((DER / calPerKg) * 1000.0) / feedingCount;
  portionGrams = grams;                          // 전역 변수에 저장

  // 디버깅 출력
  Serial.println("========================");
  Serial.println("🔬 급식량 계산 결과");
  Serial.print("체중: "); Serial.print(dogWeight); Serial.println(" kg");
  Serial.print("활동지수: "); Serial.println(activeLvl);
  Serial.print("급여 횟수: "); Serial.println(feedingCount);
  Serial.print("RER: "); Serial.print(RER); Serial.println(" kcal");
  Serial.print("DER: "); Serial.print(DER); Serial.println(" kcal");
  Serial.print("1회 사료량: "); Serial.print(grams); Serial.println(" g");
  Serial.println("========================");

  return grams;
}

// 이동 평균 기반 정밀 무게 측정 함수
float getSuperStableWeight() {
  const int numReadings = 10;
  float readings[numReadings];
  float sum = 0;

  for (int i = 0; i < numReadings; i++) {
    readings[i] = hx711_calc.get_units();
    delay(30);
  }

  for (int i = 0; i < numReadings; i++) {
    sum += readings[i];
  }
  float average = sum / numReadings;

  float filteredSum = 0;
  int filteredCount = 0;
  for (int i = 0; i < numReadings; i++) {
    if (abs(readings[i] - average) < 0.02) {
      filteredSum += readings[i];
      filteredCount++;
    }
  }

  return (filteredCount > 0) ? (filteredSum / filteredCount) : average;
}
