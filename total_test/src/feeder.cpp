// feeder.cpp
#include "feeder.h"
#include "feeding_calc.h"
#include <HX711.h>
#include <Servo.h>
#include <RTClib.h>

// ────────────────────────────────────────────────────────────────────────────
// 전역 인스턴스 정의
HX711      hx711;
Servo      servo;

// 칼만 필터 변수 (static으로 은닉)
static float kalmanEstimate = 0.0;
static float kalmanErrorCov = 1.0;
static const float kalmanQ  = 0.01;
static const float kalmanR  = 0.5;

// 허용 오차 및 안정 판정
float tolerancePercent        = 5.0;  
int   stableCount             = 0;
const int requiredStableCount = 5;

// 메디안 필터용 읽기 수
const int numReadings = 20;
// 기준 무게 저장 (g)
float baselineGross = 0.0;

// ────────────────────────────────────────────────────────────────────────────
// 퀵 정렬 (메디안 필터 지원)
static void quickSort(float arr[], int l, int r) {
  int i = l, j = r;
  float pivot = arr[(l + r) / 2];
  while (i <= j) {
    while (arr[i] < pivot) i++;
    while (arr[j] > pivot) j--;
    if (i <= j) {
      float t = arr[i]; arr[i] = arr[j]; arr[j] = t;
      i++; j--;
    }
  }
  if (l < j) quickSort(arr, l, j);
  if (i < r) quickSort(arr, i, r);
}

// 메디안 필터 → kg 단위
static float getMedianFilteredWeight() {
  float v[numReadings];
  for (int i = 0; i < numReadings; i++) {
    v[i] = hx711.get_units();
    delay(30);
  }
  quickSort(v, 0, numReadings - 1);
  int m = numReadings / 2;
  return (numReadings % 2 == 0) ? (v[m - 1] + v[m]) / 2.0 : v[m];
}

// 칼만 필터 → g 단위 입력
static float applyKalman(float meas) {
  float K = kalmanErrorCov / (kalmanErrorCov + kalmanR);
  kalmanEstimate += K * (meas - kalmanEstimate);
  kalmanErrorCov = (1 - K) * kalmanErrorCov + kalmanQ;
  return kalmanEstimate;
}

// ────────────────────────────────────────────────────────────────────────────
void initFeeder() {
  Serial.begin(115200);
  hx711.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  hx711.set_scale(calibration_factor);

  Serial.println("그릇만 올려둔 상태로 잠시 기다려주세요...");
  delay(2000);
  hx711.tare();
  Serial.println("✅ tare 완료 (그릇 무게 0점 설정)");

  servo.attach(SERVOPIN);
  servo.write(0);  // 닫기
 }

void runFeedingSchedule() {
  executeFeeding();
}

void executeFeeding() {
  Serial.println("[" + getTimeString(rtc.now()) + "] 급식 시작");

  // 1) 1회 급여량 계산 (전역 portionGrams에 저장)
  portionGrams = (feedCount, feedTimes, /* dogWeight, activeLvl, calPerKg */);
  float target = portionGrams;
  float minT   = target * (1 - tolerancePercent / 100.0);
  float maxT   = target * (1 + tolerancePercent / 100.0);

  // 2) 기준 무게 측정 (서보 닫힌 상태): g 단위
  float firstKg      = getMedianFilteredWeight();
  baselineGross      = applyKalman(firstKg * 1000.0);
  Serial.print("🟢 초기 전체 무게: ");
  Serial.print(baselineGross, 1);
  Serial.println(" g");

  // 3) 서보 열기 → 사료 배출 시작
  servo.write(90);
  delay(500);
  stableCount = 0;

  // 4) 배출량 모니터링 루프
  while (stableCount < requiredStableCount) {
    float nowKg      = getMedianFilteredWeight();
    float nowG       = applyKalman(nowKg * 1000.0);
    float dispensed  = baselineGross - nowG;

    Serial.print("Dispensed: ");
    Serial.print(dispensed, 1);
    Serial.print(" g | 안정카운트: ");
    Serial.println(stableCount);

    if (dispensed >= minT && dispensed <= maxT) stableCount++;
    else if (stableCount > 0)                stableCount--;
    delay(500);
  }

  // 5) 서보 닫기 & 플래그 설정
  servo.write(0);
  feedDoneToday[idx] = true;
  isFoodInputDone    = true;
  Serial.println("✅ 실험 완료: " + String(portionGrams, 1) + "g");
}

bool isFeedingDone() {
  return isFoodInputDone;
}
