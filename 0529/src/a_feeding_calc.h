// a_feeding_calc.h
#ifndef A_FEEDING_CALC_H
#define A_FEEDING_CALC_H

#include <Arduino.h>
#include <HX711.h>
#include "globals.h"

// 계산된 결과 전역 변수
extern float RER;             // 기초 에너지
extern float DER;             // 하루 필요 에너지
extern float portionGrams;    // 1회 급여량
extern HX711 hx711_calc;

// 사료량 계산 함수 선언
//   calPer100g: 사료 100g당 kcal
float calculatePortionGrams(int feedingCount, String feedTimes[],
                            float dogWeight, float activeLvl,
                            float calPer100g);
#endif // A_FEEDING_CALC_H
