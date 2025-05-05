#ifndef FLOWCHECK_H
#define FLOWCHECK_H

#include <Arduino.h>

void initFlowSensor();     
void pulseISR();
void flowUpdate();
float getFlowRate();
float getTotalVolume();
void resetVolume();
bool targetWater();

#endif // FLOWCHECK_H
