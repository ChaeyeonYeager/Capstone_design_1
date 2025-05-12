// feeding_calc.h
#ifndef FEEDING_CALC_H
#define FEEDING_CALC_H

#include <Arduino.h>

// 전역 저장 변수
extern float portionGrams;

// 1회 급여량 계산
float calculatePortionGrams(int   feedingCount,
                            String feedTimes[],
                            float dogWeight,
                            float activeLvl,
                            float calPerKg);

// 이동 평균 기반 정밀 무게 측정
float getSuperStableWeight();

#endif // FEEDING_CALC_H
