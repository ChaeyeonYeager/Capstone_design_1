#include <Arduino.h>
#include "feeding_grinder.h"

// 사료분쇄 핀 연결 설정
const int IN1 = 7;
const int IN2 = 6;
const int ENA = 5;  // PWM 핀

bool isGrinding = false;

void initmotorGrinder()
{
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    pinMode(ENA, OUTPUT);
    Serial.begin(9600);
    Serial.println("SZH-EK001 모터 드라이버 준비 완료");
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

bool isGrindingDone()
{
    return isGrinding;
}

// 메인루프에서는 motorGrinder()만 호출
