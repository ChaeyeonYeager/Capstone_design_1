#ifndef FEEDING_CALC_H
#define FEEDING_CALC_H

#include <Arduino.h>

// 계산된 결과 전역 변수
extern float RER;             // 기초 에너지
extern float DER;             // 하루 필요 에너지
extern float portionGrams;    // 1회 급여량

// 사료량 계산 함수 선언
float calculatePortionGrams(int feedingCount, String feedTimes[], float dogWeight, float activeLvl, float calPerKg);

#endif // FEEDING_CALC_H
