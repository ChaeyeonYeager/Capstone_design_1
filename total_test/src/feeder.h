#ifndef FEEDER_H
#define FEEDER_H

#include <HX711.h>
#include <Servo.h>

// 핀 정의
#define LOADCELL_DOUT_PIN 2
#define LOADCELL_SCK_PIN 3
#define SERVOPIN 9

// 외부 인스턴스 선언
extern HX711 hx711;
extern Servo servo;

// 설정값 (globals.cpp 등에서 정의됨)
extern float  calibration_factor;   // 로드셀 보정값
extern float  portionGrams;         // 목표 급여량 (g)
extern bool   feedDoneToday[];      // 급식 완료 여부 (PetFeeder 연동)
extern bool   isFoodInputDone;      // 전체 완료 여부

// 함수 선언
void initFeeder();                  // 하드웨어 초기화 (서보 attach)
void executeFeeding(int index);     // 정량 투입 루틴 (PetFeeder가 호출)

#endif // FEEDER_H
