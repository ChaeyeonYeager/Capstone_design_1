#ifndef PUMP_H
#define PUMP_H

#include <Arduino.h>

void initPump();      // 펌프 + 유량 센서 초기화
void pumpUpdate();    // 유량 기반 펌프 제어
bool isPumpOn();      // 펌프 상태 반환

#endif // PUMP_H
