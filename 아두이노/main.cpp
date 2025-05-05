#include <Arduino.h>
#include "PetFeeder.h"
#include "RTClib.h"

RTC_DS3231 rtc;  // 실시간 시계 모듈

const int petCount = 5;         // 반려동물 수
PetFeeder* pets[petCount];      // PetFeeder 클래스 포인터 배열

// 반려동물 이름
String petNames[5] = {"코코", "보리", "초코", "하루", "몽이"};

// 반려동물별 급식 시간 (2회씩)
String feed1[] = {"08:00", "18:00"};
String feed2[] = {"09:00", "19:00"};
String feed3[] = {"07:30", "17:30"};
String feed4[] = {"08:15", "18:15"};
String feed5[] = {"09:30", "20:00"};

// 점도 설정 배열: 0 = 1:1, 1 = 1:1.5, 2 = 1:2
int viscosityLevels[5] = {0, 1, 2, 1, 0};

/*
===================================
📌 전체 흐름 설명
===================================
1. setup():
   - 각 반려동물의 이름, 몸무게, 나이, 급식시간, 점도 등을 기반으로 PetFeeder 객체 생성
   - 하루 급식 플래그 초기화

2. loop():
   - 1초마다 현재 시간 확인 (RTC 기준)
   - 각 반려동물의 급식 시간이 되면 runFeedingRoutine() 실행
     → 사료 투입 → 물 주입 → 점도 조절 → 분쇄 → 알림
===================================
*/

void setup() {
  Serial.begin(9600);
  rtc.begin();

  resetDailyFeeding(); // 하루 급식 완료 상태 초기화

  // 반려동물 객체 생성
  pets[0] = new PetFeeder(petNames[0], 5.2, 3, 2, feed1, 1.6, 350, viscosityLevels[0]);
  pets[1] = new PetFeeder(petNames[1], 6.1, 4, 2, feed2, 1.4, 350, viscosityLevels[1]);
  pets[2] = new PetFeeder(petNames[2], 7.3, 5, 2, feed3, 1.5, 350, viscosityLevels[2]);
  pets[3] = new PetFeeder(petNames[3], 4.5, 2, 2, feed4, 1.7, 350, viscosityLevels[3]);
  pets[4] = new PetFeeder(petNames[4], 6.8, 6, 2, feed5, 1.3, 350, viscosityLevels[4]);
}

void loop() {
  DateTime now = rtc.now();
  String nowTime = getTimeString(now); // 현재 시각을 "HH:MM" 형식으로 변환

  for (int i = 0; i < petCount; i++) {
    for (int j = 0; j < pets[i]->getFeedCount(); j++) {
      // 아직 해당 시간 급식 안 했고, 지금이 그 시간이라면 실행
      if (nowTime == pets[i]->getFeedTime(j) && !pets[i]->isFeedingComplete()) {
        Serial.println("⏰ " + nowTime + " - " + petNames[i] + " 급식 시작!");
        pets[i]->runFeedingRoutine();
      }
    }
  }

  delay(1000); // 1초 간격으로 시간 체크
}
