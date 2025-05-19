// feeder.h
#ifndef FEEDER_H
#define FEEDER_H

#include <HX711.h>
#include <Servo.h>

// 최대 급식 횟수
#define MAX 6

// 핀 정의
#define LOADCELL_DOUT_PIN 2
#define LOADCELL_SCK_PIN 3
#define SERVOPIN 9

// 외부 인스턴스 선언
extern HX711      hx711;
extern Servo      servo;

// 설정값 (globals.cpp 등에 정의)
extern float  calibration_factor = -28000;  // 로드셀 보정값
extern int    feedCount;           // 급여 횟수

// feeding_calc.cpp 에서 계산 후 저장되는 전역
extern float  portionGrams;        // 1회 급여량 (g)

// 급식 완료 플래그
extern bool   isFoodInputDone;     // 급식 전체 완료 여부

// 함수 선언
void   initFeeder();                        // 하드웨어 초기화
void   runFeedingSchedule();                // 스케줄 확인 후 실행
void   executeFeeding();                    // 급식 루틴 수행
bool   isFeedingDone();                     // 완료 여부 반환

#endif // FEEDER_H
