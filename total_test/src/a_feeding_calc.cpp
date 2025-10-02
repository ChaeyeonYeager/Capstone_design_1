// feeding_calc.cpp
#include <Arduino.h>
#include "a_feeding_calc.h"
#include "b_feeder.h"  // hx711 전역 인스턴스 참조

// 전역에 저장된 포션 그램
float portionGrams = 0.0;

float calculatePortionGrams(int   feedingCount,
                            String feedTimes[],
                            float dogWeight,
                            float activeLvl,
                            float calPerKg) {
  float RER = 70 * pow(dogWeight, 0.75);
  float DER = RER * activeLvl;

  float grams = ((DER / calPerKg) * 1000.0) / feedingCount;
  portionGrams = grams;

  Serial.println(F("========================"));
  Serial.println(F("🔬 급식량 계산 결과"));
  Serial.print(F("체중: "));     Serial.print(dogWeight);    Serial.println(F(" kg"));
  Serial.print(F("활동지수: ")); Serial.println(activeLvl);
  Serial.print(F("급여 횟수: ")); Serial.println(feedingCount);
  Serial.print(F("RER: "));      Serial.print(RER);          Serial.println(F(" kcal"));
  Serial.print(F("DER: "));      Serial.print(DER);          Serial.println(F(" kcal"));
  Serial.print(F("1회 사료량: ")); Serial.print(grams);       Serial.println(F(" g"));
  Serial.println(F("========================"));

  return grams;
}

float getSuperStableWeight() {
  const int N = 20;
  float values[N];

  // 원시 측정값 수집
  for (int i = 0; i < N; i++) {
    values[i] = hx711.get_units();
    delay(30);
  }

  // 단순 이동 평균
  float sum = 0;
  for (int i = 0; i < N; i++) sum += values[i];
  float avg = sum / N;

  // 평균 편차 필터링
  float filtSum = 0;
  int   cnt     = 0;
  for (int i = 0; i < N; i++) {
    if (abs(values[i] - avg) < 0.02) {
      filtSum += values[i];
      cnt++;
    }
  }

  float result = (cnt > 0 ? filtSum / cnt : avg);
  return result * 1000.0;  // kg → g
}

