#include <Arduino.h>
#include "feeding_calc.h"

float RER;
float DER;
float portionGrams;

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
