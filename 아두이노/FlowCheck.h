#ifndef FLOWCHECK_H
#define FLOWCHECK_H

#include <Arduino.h>

// 함수 및 변수 선언

void FlowSensor(uint8_t interruptPin, float PerLiter); // 센서 초기화
void begin();               // 핀 설정 및 인터럽트 초기화
void pulseISR();            // 인터럽트 시 펄스 증가
void flowUpdate();          // 유량 업데이트
float getFlowRate();        // 현재 유량(mL/s) 반환
float getTotalVolume();     // 총 유량(mL) 반환
void resetVolume();         // 총 유량 초기화
bool targetWater();         // 목표 유량 달성 여부 확인

#endif // FLOWCHECK_H
