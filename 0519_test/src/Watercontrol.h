#ifndef WATERCONTROL_H
#define WATERCONTROL_H

#include <Arduino.h>
#include "globals.h" 

#define relayPin 10
#define flowSensorPin 11


// 전체 물 투입 프로세스 완료 플래그
extern bool isProcessDone;

// ────────────────────────────────────────────────────────────────────────────
// 시스템 초기화
// - Serial.begin
// - relayPin OUTPUT, flowSensorPin INPUT_PULLUP 설정
void initWaterSystem();

// ────────────────────────────────────────────────────────────────────────────
// @param units10ml: “10 ml 단위” 개수
//   예) runWaterProcess(1) → 10 ml, runWaterProcess(5) → 50 ml
// 내부에서 pulsePer10ml 상수와 곱해 필요한 펄스 수를 계산하여
// 펌프 ON → 목표 펄스 도달 시 OFF 후 isProcessDone = true
void runWaterProcess(int units10ml);

// ────────────────────────────────────────────────────────────────────────────
// 30분 불림 대기 (delay(1800000))
// 대기 완료 후 내부 isSoaking 플래그만 true로 올리고 리턴
void waitSoaking();

#endif // WATERCONTROL_H