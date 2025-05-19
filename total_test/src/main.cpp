#include <Arduino.h>
#include "PetFeeder.h"
#include "RTClib.h"

#include "b_calibration_routine.h"
#include "b_calibration_util.h"
#include "globals.h"

RTC_DS3231 rtc;  // 실시간 시계 모듈

const int petCount = 5;
PetFeeder* pets[petCount];

// 반려동물 이름
String petNames[5] = {"코코", "보리", "초코", "하루", "몽이"};

// 급식 시간 설정
String feed1[] = {"08:00", "18:00"};
String feed2[] = {"09:00", "19:00"};
String feed3[] = {"07:30", "17:30"};
String feed4[] = {"08:15", "18:15"};
String feed5[] = {"09:30", "20:00"};

// 점도 설정 (0 = 1:1, 1 = 1:1.5, 2 = 1:2)
int viscosityLevels[5] = {0, 1, 2, 1, 0};

/*
===================================
📌 전체 흐름 설명
===================================
1. setup():
   - 각 반려동물의 설정값 기반 PetFeeder 객체 생성
   - 급식 플래그 초기화

2. loop():
   - 매초 현재 시간 확인
   - 각 동물의 급식 시간이 도달하면 runFeedingRoutine() 실행
   - 분쇄 완료된 경우 → 30분 후 자동 잔량 체크 실행
===================================
*/

void setup() {
  Serial.begin(9600);
  rtc.begin();

  resetDailyFeeding();  // 하루 급식 플래그 초기화

  pets[0] = new PetFeeder(petNames[0], 5.2, 3, 2, feed1, 1.6, 350, viscosityLevels[0]);
  pets[1] = new PetFeeder(petNames[1], 6.1, 4, 2, feed2, 1.4, 350, viscosityLevels[1]);
  pets[2] = new PetFeeder(petNames[2], 7.3, 5, 2, feed3, 1.5, 350, viscosityLevels[2]);
  pets[3] = new PetFeeder(petNames[3], 4.5, 2, 2, feed4, 1.7, 350, viscosityLevels[3]);
  pets[4] = new PetFeeder(petNames[4], 6.8, 6, 2, feed5, 1.3, 350, viscosityLevels[4]);

  calibration_factor = loadCalibrationFactor();  // EEPROM에서 로딩
  runCalibration();  // 최초 실행 시만 호출 (혹은 조건 추가)

}

void loop() {
  DateTime now = rtc.now();
  String nowTime = getTimeString(now);  // "HH:MM"

  for (int i = 0; i < petCount; i++) {
    // ✅ 급식 시간이면 루틴 실행
    for (int j = 0; j < pets[i]->getFeedCount(); j++) {
      if (nowTime == pets[i]->getFeedTime(j) && !pets[i]->isFeedingComplete()) {
        Serial.println("⏰ " + nowTime + " - " + petNames[i] + " 급식 시작!");
        pets[i]->runFeedingRoutine();
      }
    }

    // ✅ 분쇄 후 30분 지났는지 체크 → 잔량 측정
    pets[i]->checkFoodLevelAfterGrindDelay();
  }

  delay(1000);  // 1초 간격 주기
}
