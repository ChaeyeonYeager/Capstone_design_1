#ifndef PUMP_H
#define PUMP_H

#include <Arduino.h>

// 함수 선언
void pumpUpdate();     // 유량 상태를 기반으로 펌프 제어
bool isPumpOn();       // 현재 펌프 상태 반환
void setupPump();      // 펌프 및 센서 초기화

#endif // PUMP_H
