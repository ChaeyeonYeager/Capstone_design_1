#ifndef B_FEEDER_H
#define B_FEEDER_H

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
// setup()에서 한 번만 호출
void initFeeder();

// 정량 배출 (파라미터 없음)
void executeFeeding();

#endif // FEEDER_H
