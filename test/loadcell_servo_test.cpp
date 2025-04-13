#include <HX711.h>      // 로드셀 라이브러리
#include <Servo.h>      // 서보모터 라이브러리
#include <Wire.h>
#include "RTClib.h"     // RTC 모듈 라이브러리

// ======= 핀 설정 =======
#define DOUT_PIN A1      // HX711 DOUT
#define CLK_PIN  A0      // HX711 CLK
#define SERVO_PIN 9      // 서보모터 제어 핀

// ======= 급식 시간 설정 =======
#define FEED_HOUR 12     // 원하는 시간 설정 (12시 00분)
#define FEED_MINUTE 0

// ======= 급식 기준 무게(g) =======
#define TARGET_WEIGHT 30.0       // 목표 사료 g
#define TOLERANCE 0.05           // ±5% 허용 오차

HX711 scale;
Servo servo;
RTC_DS3231 rtc;

bool hasFed = false; // 오늘 급식 여부

void setup() {
  Serial.begin(9600);

  scale.begin(DOUT_PIN, CLK_PIN);
  scale.set_scale(2280.f); // 보정 필요
  scale.tare();

  servo.attach(SERVO_PIN);
  servo.write(0); // 닫힌 상태로 시작

  if (!rtc.begin()) {
    Serial.println("RTC 연결 실패");
    while (1);
  }

  if (rtc.lostPower()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); // 현재 시간으로 초기화
  }
}

void loop() {
  DateTime now = rtc.now();
  int hour = now.hour();
  int minute = now.minute();

  // 하루가 지나면 다시 급식 가능하도록 리셋
  if (hour == 0 && minute == 0) {
    hasFed = false;
  }

  // 급식 시간 도달 + 아직 급식 안 했으면
  if (!hasFed && hour == FEED_HOUR && minute == FEED_MINUTE) {
    Serial.println("급식 시간 도달! 문 열림");
    servo.write(90); // 문 열기
    delay(3000);     // 사료 떨어질 시간

    float minWeight = TARGET_WEIGHT * (1.0 - TOLERANCE);
    float maxWeight = TARGET_WEIGHT * (1.0 + TOLERANCE);
    float measured = 0;

    // 로드셀 감시: 목표 무게 도달하면 문 닫기
    while (true) {
      measured = scale.get_units();
      Serial.println("현재 무게: " + String(measured, 1) + "g");
      if (measured >= minWeight && measured <= maxWeight) {
        break;
      }
      delay(200);
    }

    servo.write(0); // 문 닫기
    Serial.println("목표 무게 도달 → 문 닫힘");
    hasFed = true;
  }

  delay(1000);
}
