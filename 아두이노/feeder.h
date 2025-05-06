#ifndef FEEDER_H
#define FEEDER_H

#include <HX711.h>
#include <Wire.h>
#include "RTClib.h"
#include <Servo.h>

// 최대 급식 횟수 정의
#define MAX 6

// 아두이노 핀 
#define LOADCELL_DOUT_PIN 2
#define LOADCELL_SCK_PIN 3
#define SERVOPIN 9

// 외부에서 사용할 하드웨어 인스턴스
extern HX711 hx711;
extern RTC_DS3231 rtc;
extern Servo servo;

// 설정값들 (앱 또는 초기 설정)
float calibration_factor = -28000;        // 로드셀 보정값extern int feedCount;                     // 급여 횟수
float plate_weight = 1012.0;              // 그릇 무게 (g)  
extern String feedTimes[MAX];             // 급여 시간 (HH:MM 형식)
extern float portionGrams;                // 1회 급여량 (g)
extern bool feedDoneToday[MAX];           // 해당 시간 급여 완료 여부
extern bool isFoodInputDone;              // 급식 전체 완료 여부

// 함수 선언
void initFeeder();                        // 초기화 함수
void runFeedingSchedule();                // 시간 확인 후 급식 수행
void executeFeeding(int index);           // 실제 급식 수행
void resetDailyFeeding();                 // 하루 시작 시 플래그 초기화
bool isFeedingDone();                     // 급식 완료 여부 반환
String getTimeString(DateTime now);       // 시간 포맷 변환환

#endif


// - 급식 스케줄러 함수 runFeedingSchedule() 구현
// - 로드셀 기반 사료 투입 로직 정비 (95~105% 허용)
// - 하루 초기화 함수 resetDailyFeeding() 정리
