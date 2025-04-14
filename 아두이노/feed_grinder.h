#ifndef FEED_GRINDER_H
#define FEED_GRINDER_H

// 사료분쇄 핀 연결 설정
const int IN1 = 7;
const int IN2 = 6;
const int ENA = 5;  // PWM 핀

bool isGrinding = false;

void initmotorGrinder();
void motorGrinder();
void rotateMotor(int speed);
void stopMotor();
bool isGrindingDone();
#endif
