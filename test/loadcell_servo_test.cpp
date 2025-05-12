#include <HX711.h>
#include <Servo.h>

HX711 hx711;
Servo myServo;

// ────────────────────────────────────────────────────────────────────────────
// 칼만 필터 변수
float kalmanEstimate = 0.0;
float kalmanErrorCov = 1.0;
const float kalmanQ = 0.01;
const float kalmanR = 0.5;
// ────────────────────────────────────────────────────────────────────────────

float calibration_factor = -30000; // 초기 보정값
float tolerancePercent    = 5.0;   // 허용 오차 ±5%
bool opened               = false;

#define LOADCELL_DOUT_PIN 2
#define LOADCELL_SCK_PIN 3
#define SERVOPIN         9

#define goal_weight    40.0  // 목표 사료 무게 (g)
#define plate_weight    0.0  // tare() 모드: 그릇 무게는 0점

int stableCount             = 0;
const int requiredStableCount = 5;

// 실험용 기준 무게 저장
float baselineGross = 0.0;

// 퀵 정렬 (메디안 필터용)
void quickSort(float arr[], int left, int right) {
  int i = left, j = right;
  float pivot = arr[(left + right) / 2];
  while (i <= j) {
    while (arr[i] < pivot) i++;
    while (arr[j] > pivot) j--;
    if (i <= j) {
      float tmp = arr[i]; arr[i] = arr[j]; arr[j] = tmp;
      i++; j--;
    }
  }
  if (left < j)  quickSort(arr, left, j);
  if (i < right) quickSort(arr, i, right);
}

// 중앙값 기반 무게 측정 (kg 단위)
float getMedianFilteredWeight() {
  const int numReadings = 20;
  float readings[numReadings];
  for (int i = 0; i < numReadings; i++) {
    readings[i] = hx711.get_units();
    delay(30);
  }
  quickSort(readings, 0, numReadings - 1);
  if (numReadings % 2 == 0) {
    int mid = numReadings / 2;
    return (readings[mid - 1] + readings[mid]) / 2.0;
  } else {
    return readings[numReadings / 2];
  }
}

// 칼만 필터 업데이트 (g 단위 입력)
float applyKalman(float measurement) {
  float K = kalmanErrorCov / (kalmanErrorCov + kalmanR);
  kalmanEstimate += K * (measurement - kalmanEstimate);
  kalmanErrorCov = (1 - K) * kalmanErrorCov + kalmanQ;
  return kalmanEstimate;
}

void setup() {
  Serial.begin(115200);
  hx711.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  hx711.set_scale(calibration_factor);
  myServo.attach(SERVOPIN);
  myServo.write(90); // 서보 초기 닫기

  // 1) tare 자동 수행
  Serial.println("그릇만 올려둔 상태로 잠시 기다려주세요...");
  delay(2000);
  hx711.tare();
  Serial.println("✅ tare 완료 (그릇 무게 0점 설정)");

  // 2) 자동 캘리브레이션
  Serial.println("40 g 표준추를 올려주세요. 5초 후 자동 보정 시작...");
  delay(5000);
  float rawKg     = getMedianFilteredWeight();
  float measGrams = rawKg * 1000.0;
  calibration_factor *= (measGrams / goal_weight);
  hx711.set_scale(calibration_factor);
  Serial.print("🔧 자동 보정된 팩터: ");
  Serial.println(calibration_factor);
  Serial.println("자동 보정 완료. 이제 사료를 부어주세요.");
}

void loop() {
  // 1) 메디안+칼만 필터로 현재 그릇+사료 무게 측정 (g)
  float rawKg       = getMedianFilteredWeight();
  float grossWeight = rawKg * 1000.0;
  float filtered    = applyKalman(grossWeight);

  // 2) 초기 기준 무게 측정 및 서보 개방
  if (!opened) {
    baselineGross = filtered;
    myServo.write(0);    // 서보 열기
    delay(1000);
    Serial.print("🟢 사료통 초기 무게: "); Serial.print(baselineGross, 1); Serial.println(" g");
    Serial.println("사료 배출 시작!");
    opened = true;
    stableCount = 0;
    return;
  }

  // 3) 배출된 사료량 계산
  float dispensed = baselineGross - filtered;

  // 4) 안정 판정 및 디버그 출력
  Serial.print("Dispensed: "); Serial.print(dispensed, 1);
  Serial.print(" g | 안정카운트: "); Serial.println(stableCount);

  float minT = goal_weight * (1.0 - tolerancePercent/100.0);
  float maxT = goal_weight * (1.0 + tolerancePercent/100.0);

  if (dispensed >= minT && dispensed <= maxT) {
    stableCount++;
  } else if (stableCount > 0) {
    stableCount--;
  }

  // 5) 목표량 충족 시 서보 닫기 및 종료
  if (stableCount >= requiredStableCount) {
    Serial.println("✅ 목표량 안정 충족! 서보 닫음.");
    myServo.write(90);
    while (true);  // 실험 종료: 무한 대기
  }

  delay(500);
}
