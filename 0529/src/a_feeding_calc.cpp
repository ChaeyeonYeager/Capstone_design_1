// a_feeding_calc.cpp
#include <Arduino.h>
#include <a_feeding_calc.h>
#include "globals.h"
#include <math.h> // pow(), abs()

float calculatePortionGrams(int feedingCount, String feedTimes[],
                            float dogWeight, float activeLvl,
                            float calPer100g) {
  // 1) 에너지 요구량 계산
  RER = 70 * pow(dogWeight, 0.75);    // 기초 에너지(kcal)
  DER = RER * activeLvl;              // 활동 반영 하루 에너지(kcal)

  // 2) 100g당 칼로리를 kg당 칼로리로 환산
  float calPerKgFeed = calPer100g * 10.0;  // ex. 350 → 3500 kcal/kg

  // 3) 1회 급여량(g) = (DER / calPerKgFeed) * 1000g ÷ 급여횟수
  float grams = ((DER / calPerKgFeed) * 1000.0) / feedingCount;
  //portionGrams = grams;  // 전역 변수에 저장

  // 4) 디버깅 출력
  Serial.println(F("========================"));
  Serial.println(F("🔬 급식량 계산 결과"));
  Serial.print(F("체중: "));       Serial.print(dogWeight, 2); Serial.println(F(" kg"));
  Serial.print(F("활동량 계수: ")); Serial.println(activeLvl, 2);
  Serial.print(F("급여 횟수: "));   Serial.println(feedingCount);
  Serial.print(F("RER: "));         Serial.print(RER, 1);       Serial.println(F(" kcal"));
  Serial.print(F("DER: "));         Serial.print(DER, 1);       Serial.println(F(" kcal"));
  Serial.print(F("사료 칼로리: ")); Serial.print(calPer100g);   Serial.println(F(" kcal/100g"));
  Serial.print(F("1회 사료량: "));   Serial.print(grams, 1);     Serial.println(F(" g"));
  Serial.println(F("========================"));

  return grams;
}

