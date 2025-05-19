#include <Arduino.h>
#include "PetFeeder.h"
#include "RTClib.h"

RTC_DS3231 rtc;  // 실시간 시계 모듈

// 테스트용 급식 시간 설정 (feedCount = 2)
String testFeedTimes[] = {"08:00", "18:00"};
PetFeeder* testPet;

// 시간 문자열 변환 헬퍼
String getTimeString(const DateTime& dt) {
  char buf[6];
  snprintf(buf, sizeof(buf), "%02d:%02d", dt.hour(), dt.minute());
  return String(buf);
}

void setup() {
  Serial.begin(9600);
  rtc.begin();
  initFeeder();
  calibrateFeeder();
  testPet = new PetFeeder(
    "코코", 5.2, 3, 2, testFeedTimes, 1.6, 350, 1
  );
}

void loop() {
  DateTime now   = rtc.now();
  String  nowStr = getTimeString(now);
  int     today  = now.day();

  // 1) 날짜가 바뀌면 플래그 초기화
  static int  lastDay   = -1;
  static bool fedFlags[2] = {false, false};
  if (today != lastDay) {
    lastDay    = today;
    fedFlags[0] = fedFlags[1] = false;
  }

  // 2) 각 급식 시간마다 동작
  for (int j = 0; j < testPet->getFeedCount(); j++) {
    if (nowStr == testFeedTimes[j] && !fedFlags[j]) {
      Serial.println("⏰ " + nowStr + " - 코코 급식 시작!");
      testPet->runFeedingRoutine();
      fedFlags[j] = true;  // 한 번 실행됐다고 표시
    }
  }

  delay(1000);
}
