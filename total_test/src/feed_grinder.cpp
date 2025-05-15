#include <Arduino.h>
#include "feed_grinder.h"

void initmotorGrinder()
{
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    pinMode(ENA, OUTPUT);
    pinMode(speakerPin, OUTPUT);
}

// ✅ 모터 동작 순서 함수
void motorGrinder() {
  Serial.println("🚗 천천히 회전 시작");
  rotateMotor(100);   // 천천히 회전
  delay(5000);        // 5초 유지

  Serial.println("🚀 최대 속도 회전 시작");
  rotateMotor(255);   // 최대 속도 회전
  delay(10000);       // 10초 유지

  Serial.println("🛑 정지");
  stopMotor();        // 모터 정지
  delay(5000);        // 정지 상태 유지

  alertFor3Seconds(); //


  isGrinding = true;
}

// ✅ 정방향 회전 함수 (속도 지정)
void rotateMotor(int speed) {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, speed);
}

// ✅ 정지 함수
void stopMotor() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, 0);
}

// ✅ 알림 함수 (3초간 삐 소리 - 수동 부저용)
void alertFor3Seconds() {
  tone(speakerPin, 1000);   // 1000Hz의 소리를 핀 4번으로 출력
  delay(3000);     // 3초 동안 울림
  noTone(speakerPin);       // 소리 끔
}

bool isGrindingDone()
{
    return isGrinding;
}

// 메인루프에서는 motorGrinder()만 호출
