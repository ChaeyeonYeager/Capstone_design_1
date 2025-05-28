#ifndef C_FOODSOAKING_H
#define C_FOODSOAKING_H


#include <Arduino.h>

// 1:1 비율로 불림용 물 주입 후 30분 대기
// @param baseWaterVolume  : 1회 불림용 물 부피(ml)
void soakFoodProcess(float baseWaterVolume);


#endif // FOODSOAKING_H