// Feeder.h - 자동 급식기 제어 클래스 헤더 파일
#ifndef FEEDER_H
#define FEEDER_H

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
    string feedTimes[6];           // 최대 6회 급식 시간
    float activityLevel;
    int kcalPerKg;
    float portionGrams;
    bool feedDoneToday[6];        // 각 시간별 급식 완료 여부
    float activityFactor;
    bool isFoodInputDone;
  
  void checkAndFeed();  // 자동 급식 처리 함수
  String getTimeString(DateTime now); // "HH:MM" 시간 형식 반환
  void feedPortion(int index);  // 사료 급여 함수(로드셀, 서보모터 동작)
  void resetFeedingFlags();     // 하루 급식 플래그 초기화
  bool isFoodInputDoneState(); // 배급 상태 확인 함수

#endif
