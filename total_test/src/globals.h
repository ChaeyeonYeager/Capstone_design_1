#ifndef GLOBALS_H
#define GLOBALS_H
#include <Arduino.h>
#include <RTClib.h>

// 최대 사료 스케줄 수 (예: 하루 5마리 × 2회)
#define MAX_FEEDS 10

// 급식 완료 플래그
extern bool feedDoneToday[MAX_FEEDS];
extern bool isFoodInputDone;

// 하루 플래그 초기화
void resetDailyFeeding();

// RTC DateTime을 "HH:MM" 문자열로 변환
String getTimeString(const DateTime& dt);


// 📌 전역 보정값 (모든 모듈에서 사용 가능)
extern float calibration_factor;

// 📌 EEPROM 저장/불러오기 함수 선언
void loadCalibrationFromEEPROM();
void saveCalibrationToEEPROM(float f);

#endif
