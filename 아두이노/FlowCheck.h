#ifndef FLOWCHECK_H
#define FLOWCHECK_H

#include <Arduino.h>

void initFlowSensor();      // 매개변수 없는 초기화
void pulseISR();
void flowUpdate();
float getFlowRate();
float getTotalVolume();
void resetVolume();
bool targetWater();

#endif // FLOWCHECK_H
