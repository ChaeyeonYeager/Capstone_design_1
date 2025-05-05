// Feeder.h - 자동 급식기 제어 클래스 헤더 파일
#ifndef FEEDER_H
#define FEEDER_H
#define MAX 6

#include <HX711.h>
#include <Wire.h>
#include "RTClib.h"
#include <Servo.h>
#include <SoftwareSerial.h>

  HX711 scale;
  RTC_DS3231 rtc;
  Servo servo;
  SoftwareSerial BT;

    // 애플리케이션에서 전달받을 설정값들
    string petName;
    int age;
    float weight;
    int feedCount;
    string feedTimes[MAX];           // 최대 max회 급식 시간
    float activityLevel;
    int kcalPerKg;
    float portionGrams;
    bool feedDoneToday[MAX];        // 각 시간별 급식 완료 여부
    float activityFactor;
    bool isFoodInputDone;
  
// === 함수 선언 ===
void runFeedingSchedule();             // 급식 시간 체크 후 자동 급식
void executeFeeding(int index);        // 실제 급식 수행 (서보 + 로드셀)
void resetDailyFeeding();              // 하루 시작 시 플래그 초기화
bool isFeedingDone();                  // 급식 완료 여부 반환
String getTimeString(DateTime now);    // "HH:MM" 형식 시간 반환

#endif
