#ifndef FEED_GRINDER_H
#define FEED_GRINDER_H

// 사료분쇄 핀 연결 설정
const int IN1 = 7;
const int IN2 = 6;
const int ENA = 5;  // PWM 핀
const int speakerPin = 5; // 스피커의 S 핀 연결된 디지털 핀

bool isGrinding = false;

void initmotorGrinder();
void motorGrinder();
void rotateMotor(int speed);
void stopMotor();
void alertFor3Seconds();
bool isGrindingDone();
#endif
