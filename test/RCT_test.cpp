// 임의로 설정한 시간에 알람 메시지 출력만 확인
#include <Wire.h>
#include "RTClib.h"

// ===================== 객체 생성 =====================
RTC_DS3231 rtc;

// ===================== 알람 시간 설정 =====================
String alarmTime = "15:30"; // 테스트용 알람 시간 설정 ("HH:MM" 형식)

void setup() {
  Serial.begin(9600);
  rtc.begin();

  if (rtc.lostPower()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); // 컴파일 시간 기준으로 설정
  }

  Serial.println("⏰ RTC 테스트 시작!");
}

void loop() {
  checkAlarm(); // loop에서는 이 함수만 호출
}

// ===================== 알람 체크 함수 =====================
void checkAlarm() {
  DateTime now = rtc.now();
  String currentTime = getTimeString(now);

  Serial.println("현재 시간: " + currentTime);

  if (currentTime == alarmTime) {
    Serial.println("🔔 알람 시간 도달!");
    delay(60000); // 1분 동안 다시 알람 안 울리게
  }

  delay(1000);
}

// ===================== 시간 포맷 변환 =====================
String getTimeString(DateTime now) {
  char buf[6];
  sprintf(buf, "%02d:%02d", now.hour(), now.minute());
  return String(buf);
}
