#ifndef F_FEED_LEVEL_CHECK_H
#define F_FEED_LEVEL_CHECK_H

#include <Arduino.h>

// 함수 선언
void initFeedingSystem();
void handleFeedingLogic();
void checkFoodLevel();
float getAverageDistance();
float measureDistance(int trig, int echo, const char* label);

// 외부에서 제어할 수 있게 변수도 선언 (예: feeding_done)
extern bool feeding_done;

#endif
