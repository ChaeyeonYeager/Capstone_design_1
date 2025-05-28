#ifndef CALIBRATION_ROUTINE_H
#define CALIBRATION_ROUTINE_H

#include <Arduino.h>
#include <HX711.h>

// calibration_routine.cpp 에서만 정의
extern HX711 scale;
extern float calibration_factor;
extern float containerWeight;

// 0점 보정: HX711 tare & 알려진 컨테이너 무게 설정
void calibrateZero();

// 컨테이너 보정: raw 평균으로 scale factor 계산
void calibrateContainer();

#endif // CALIBRATION_ROUTINE_H
