// src/b_feeder.h
#ifndef B_FEEDER_H
#define B_FEEDER_H

#include <Arduino.h>
#include <HX711.h>
#include <Servo.h>
#include "globals.h"
#include "pinnum.h"


// 하드웨어 초기화
void initFeeder();

// PetFeeder 에서 계산한 1회 사료량을 여기에 설정
extern float feedTarget;

// feedTarget(g)만큼 사료를 투여하는 루틴
void feedFoodProcess();

#endif // B_FEEDER_H
