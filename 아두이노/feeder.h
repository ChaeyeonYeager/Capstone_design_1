// Feeder.h - 자동 급식기 제어 클래스 헤더 파일
#ifndef FEEDER_H
#define FEEDER_H

#include <HX711.h>
#include <Wire.h>
#include "RTClib.h"
#include <Servo.h>
#include <SoftwareSerial.h>

class Feeder {
public:
  Feeder(int doutPin, int clkPin, int servoPin, int rxPin, int txPin);

  void setup();                  // 초기화 함수
  void loop();                   // 메인 루프에서 호출할 함수

private:
  // 객체
  HX711 scale;
  RTC_DS3231 rtc;
  Servo servo;
  SoftwareSerial BT;

  // 애플리케이션에서 전달받을 설정값들
  String petName;
  int age;
  float weight;
  int feedCount;
  String feedTimes[6];           // 최대 6회 급식 시간
  String activityLevel;
  int kcalPerKg;
  float portionGrams;
  bool feedDoneToday[6];        // 각 시간별 급식 완료 여부
  float activityFactor;
  bool isFoodInputDone;

  // 내부 함수
  void receiveBluetoothData();  // 블루투스 데이터 수신
  void parseBluetoothData(String input); // 수신된 데이터 파싱
  void calculatePortion();      // 1회 사료량 계산
  String getTimeString(DateTime now); // "HH:MM" 시간 형식 반환
  void feedPortion(int index);  // 사료 급여 함수
  void resetFeedingFlags();     // 하루 급식 플래그 초기화
  bool isFoodInputDoneStatus(); //상태확인 함수 
};

#endif
