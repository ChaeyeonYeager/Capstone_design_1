#ifndef WATER_CONTROL_H
#define WATER_CONTROL_H

#include <Arduino.h>

// 핀 설정
const int pumpPin = 8;
const int flowSensorPin = 6;

// 100ml 기준으로 측정된 펄스 수 (CheckFlowPulse에서 얻은 값)
const int targetPulseCount = 300; // 예: 100ml 기준 펄스 수
extern volatile int flowPulseCount;

// 함수 선언
void initWaterSystem();
void startWaterInjection();
void waitForSoaking();
void countFlowPulse();

#endif
