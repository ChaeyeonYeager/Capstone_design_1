#include "b_feeder.h"
#include "calibration_routine.h"
#include <Servo.h>
#include <math.h>
#include <string.h>

static Servo servo;
float feedTarget = 0.0f;

// 칼만 필터 (반응속도 향상용)
static float kalman_est = 0.0f, kalman_cov = 1.0f;
static const float kalman_q = 0.05f, kalman_r = 0.5f;

// 메디안 필터 설정
static const int MEDIAN_WINDOW = 7;
static float median_buf[MEDIAN_WINDOW];
static int median_idx = 0;

static float kalmanFilter(float v) {
  float pred_e = kalman_est;
  float pred_c = kalman_cov + kalman_q;
  float K = pred_c / (pred_c + kalman_r);
  kalman_est = pred_e + K * (v - pred_e);
  kalman_cov = (1 - K) * pred_c;
  return kalman_est;
}

static void sortArray(float* a, int n) {
  for (int i = 1; i < n; i++) {
    float key = a[i]; int j = i - 1;
    while (j >= 0 && a[j] > key) {
      a[j + 1] = a[j];
      j--;
    }
    a[j + 1] = key;
  }
}

static float medianFilter(float v) {
  median_buf[median_idx++] = v;
  if (median_idx >= MEDIAN_WINDOW) median_idx = 0;
  float tmp[MEDIAN_WINDOW];
  memcpy(tmp, median_buf, sizeof(tmp));
  sortArray(tmp, MEDIAN_WINDOW);
  return tmp[MEDIAN_WINDOW / 2];
}

void initFeeder() {
  scale.begin(doutPin, sckPin);
  servo.attach(servoPin);
  servo.write(0);
}

void feedFoodProcess() {
  Serial.println(F("▶ 컨테이너 안정화 중..."));

  const int HISTORY = 10;
  float history[HISTORY] = {0};
  int hIdx = 0;
  float lastFiltered = 0, raw = 0;
  float stableWeight = 0;

  while (true) {
    raw = scale.get_units(3);
    float weight = raw / calibration_factor;
    float filtered = medianFilter(kalmanFilter(weight));

    Serial.print(F("📡 실시간 무게: "));
    Serial.print(filtered, 2);
    Serial.println(F(" g"));

    history[hIdx++] = fabs(filtered - lastFiltered);
    if (hIdx >= HISTORY) hIdx = 0;

    lastFiltered = filtered;

    float deltaSum = 0;
    for (int i = 0; i < HISTORY; ++i) deltaSum += history[i];
    float avgDelta = deltaSum / HISTORY;

    if (avgDelta < 0.3f && fabs(filtered - containerWeight) < 5.0f && filtered > 1.0f) {
      stableWeight = filtered;

      containerWeight = stableWeight;
      calibration_factor = raw / containerWeight;

      Serial.println(F("🔧 보정계수 자동 업데이트됨"));
      Serial.print(F("📦 새 컨테이너 무게: "));
      Serial.println(containerWeight, 2);
      Serial.print(F("🧮 새 보정계수: "));
      Serial.println(calibration_factor, 4);

      break;
    }

    delay(80);
  }

  Serial.print(F("✅ 컨테이너 안정화 완료! 사료통 무게: "));
  Serial.print(stableWeight, 2);
  Serial.println(F(" g"));

  float currentWeight = stableWeight;
  float delivered = 0.0f;

  Serial.print(F("▶ 목표 투여량: "));
  Serial.print(feedTarget, 1);
  Serial.println(F(" g"));

  while (true) {
    Serial.println(F("🚪 서보 열림 (0.5초간)"));
    servo.write(50);
    delay(500);
    servo.write(0);
    Serial.println(F("🚪 서보 닫힘, 안정화 대기 중..."));

    float sum = 0;
    float readings[HISTORY];
    bool allLower = true;

    for (int i = 0; i < HISTORY; ++i) {
      raw = scale.get_units(3);
      float now = medianFilter(kalmanFilter(raw / calibration_factor));
      readings[i] = now;
      sum += now;

      if (now > stableWeight + 0.3f) {
        allLower = false;
      }

      delay(100);
    }

    float avg = sum / HISTORY;
    currentWeight = avg;

    // 잘못된 하강 측정값 필터링
    if (avg < stableWeight - 0.5f || allLower) {
      Serial.println(F("⚠️ 측정값이 기준보다 낮거나 감소 추세 → 무시하고 재측정"));
      continue;
    }

    delivered = currentWeight - stableWeight;

    Serial.print(F("📈 누적 투여량: "));
    Serial.print(delivered, 2);
    Serial.println(F(" g"));

    float error = fabs(delivered - feedTarget) / feedTarget * 100.0f;
    if (error <= 0.5f || delivered >= feedTarget * 0.95f) {
      Serial.println(F("✅ 목표 투여량 도달 → 서보 닫힘"));
      servo.write(0);
      break;
    }
  }
}
