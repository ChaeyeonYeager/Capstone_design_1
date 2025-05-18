#include "b_feeder.h"
#include <HX711.h>
#include <Servo.h>

// 외부 인스턴스
HX711 hx711;
Servo servo;

// 정밀 측정용 필터
static float kalmanEstimate = 0.0;
static float kalmanErrorCov = 1.0;
static const float kalmanQ  = 0.01;
static const float kalmanR  = 0.5;

float baselineGross = 0.0;
float tolerancePercent = 5.0;  // 오차 허용

// -------------------------------
// 칼만 필터
static float applyKalman(float meas) {
  float K = kalmanErrorCov / (kalmanErrorCov + kalmanR);
  kalmanEstimate += K * (meas - kalmanEstimate);
  kalmanErrorCov = (1 - K) * kalmanErrorCov + kalmanQ;
  return kalmanEstimate;
}

// -------------------------------
// 측정값 평균 (10회)
static float getAverageWeight() {
  float total = 0;
  for (int i = 0; i < 10; i++) {
    total += hx711.get_units();
    delay(30);
  }
  return total / 10.0;
}

// -------------------------------
// 초기화 (setup에서 호출됨)
void initFeeder() {
  servo.attach(SERVOPIN);
  servo.write(0);
}

// -------------------------------
// 정량 배출 루틴 (PetFeeder에서 호출)
static int _feedRunCount = 0;  // 실행 횟수 세는 내부 카운터

void executeFeeding() {
  int idx = _feedRunCount++;   // 1st call → idx=0, 2nd → idx=1, …

  Serial.println("▶ 정량 사료 배출 시작");

  float initialKg    = getAverageWeight();
  float baselineG    = applyKalman(initialKg * 1000.0);
  float minTargetG   = portionGrams * (1.0 - tolerancePercent/100.0);
  float maxTargetG   = portionGrams * (1.0 + tolerancePercent/100.0);

  Serial.print("📌 현재 무게: ");
  Serial.print(baselineG, 1);
  Serial.println(" g");

  servo.write(90);
  Serial.println("🔓 서보모터 열림 → 사료 낙하 중...");

  while (true) {
    float currentKg = getAverageWeight();
    float curG      = applyKalman(currentKg * 1000.0);
    float diff      = baselineG - curG;

    Serial.print("Dispensed: ");
    Serial.print(diff, 1);
    Serial.println(" g");

    if (diff >= minTargetG && diff <= maxTargetG) {
      servo.write(0);
      Serial.println("🔒 목표량 도달 → 서보모터 닫힘");
      break;
    }
    delay(500);
  }

  // 호출 횟수(idx)에 맞춰 플래그 세팅
  feedDoneToday[idx]   = true;
  isFoodInputDone      = true;
  Serial.println("✅ 정량 급식 완료");
}