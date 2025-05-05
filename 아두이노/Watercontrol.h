#ifndef WATERCONTROL_H
#define WATERCONTROL_H

#include <Arduino.h>

const int pumpPin = 8;
const int flowSensorPin = 2;
const int targetPulseCount = 45; // 예: 100mL 기준

void initWaterSystem();      // 시스템 초기화
void runWaterProcess();      // 물 주입 + 불림 통합 처리
bool isSoakingDone();        // 불림 완료 여부
void countFlowPulse();       // 인터럽트용 함수

#endif
