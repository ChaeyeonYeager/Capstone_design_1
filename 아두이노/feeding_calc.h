#ifndef FEED_CALC_H
#define FEED_CALC_H

#include <Arduino.h>

// ✅ 변수 선언
int feedingCount;           // 하루 급여 횟수
float dogWeight;            // 강아지 체중 (kg)
float activeLvl;            // 활동지수
float calPerKg;             // 1kg당 사료 칼로리 (kcal)

// ✅ 결과 변수
float RER;                  // 기초 에너지 요구량 (kcal)
float DER;                  // 하루 에너지 요구량 (kcal)
float foodWeightPerMeal;    // 1회 급여 사료량 (g)

  // 🔢 랜덤 테스트값 입력
  feedingCount = 2;           // 하루 2회 급여
  dogWeight = random(30, 80) / 10.0; // 3.0kg ~ 7.9kg 사이 무작위 체중
  activeLvl = 1.6;            // 중성화된 성견
  calPerKg = 3600;            // 1kg당 사료 칼로리 (예: 3600 kcal/kg)

// ✅ 함수 선언
int foodWeightPerMeal_calc(int feedingCount, float dogWeight, float activeLvl, float calPerKg); // 사료량 계산 함수

  #endif // FEED_GRINDER_H
