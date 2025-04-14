#ifndef FEED_GRINDER_H
#define FEED_GRINDER_H

#include <Arduino.h>

// 사료분쇄 핀 연결 설정
const int IN1 = 7;
const int IN2 = 6;
const int ENA = 5;  // PWM 핀

//사료분쇄 함수 선언
bool initFeedGrinder();
void rotateMotor(int speed);
void feedGrinder();
void stopMotor();


#endif // FEED_GRINDER_H
