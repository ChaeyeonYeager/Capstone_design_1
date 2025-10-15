#include <Arduino.h>
#include <Servo.h>
#include <HX711.h>
#include <math.h>

// -------------------------------
// 핀 설정
// -------------------------------
const int SERVO_PIN = 10;
const uint8_t HX_DT  = 3;
const uint8_t HX_SCK = 2;

Servo servo;
HX711 hx711;

// -------------------------------
// 로드셀 보정값 (하드코딩)
// -------------------------------
const uint32_t CAL_OFFSET = 4294672803;
const float    CAL_SCALE  = -797.160888;

// -------------------------------
// 전역 변수
// -------------------------------
float RER = 0, DER = 0, portionGrams = 0;
bool feedingActive = false;  
unsigned long t_motor = 0;  // ✅ 모터 최초 동작 시각 기록용
const float TOLERANCE = 2.0;  // g 오차 허용치
const float BOWL_WEIGHT = 100.0;  // ✅ 그릇 무게(g) 자동 보정

// -------------------------------
// 함수 선언
// -------------------------------
float calculatePortionGrams(int feedingCount, float dogWeight, float activeLvl, float calPerKg);
float getSuperStableWeight();
void runServoOnce();
void handleSerial();
void performFeeding(float targetGrams);

// -------------------------------
// setup
// -------------------------------
void setup() {
  Serial.begin(115200);
  while (!Serial) {}

  servo.attach(SERVO_PIN);
  delay(300);

  hx711.begin(HX_DT, HX_SCK);
  hx711.set_offset(CAL_OFFSET);
  hx711.set_scale(CAL_SCALE);
  hx711.tare();  // ✅ 시작 시 영점 보정

  Serial.println("\n[START] 자동 급식 시스템 준비 완료");
  Serial.println("Python에서 (이름,점수,체급,체중,활동수준,칼로리,급여횟수) 수신 대기 중...");
}

// -------------------------------
// loop
// -------------------------------
void loop() {
  handleSerial();
}

// -------------------------------
// 시리얼 수신 처리
// -------------------------------
void handleSerial() {
  if (!Serial.available()) return;

  String data = Serial.readStringUntil('\n');
  data.trim();
  if (data.length() == 0) return;

  Serial.print("[RECV] "); Serial.println(data);

  // CSV 파싱
  int idx1 = data.indexOf(',');
  int idx2 = data.indexOf(',', idx1 + 1);
  int idx3 = data.indexOf(',', idx2 + 1);
  int idx4 = data.indexOf(',', idx3 + 1);
  int idx5 = data.indexOf(',', idx4 + 1);
  int idx6 = data.indexOf(',', idx5 + 1);

  String name       = data.substring(0, idx1);
  float score       = data.substring(idx1 + 1, idx2).toFloat();
  String size       = data.substring(idx2 + 1, idx3);
  float weight      = data.substring(idx3 + 1, idx4).toFloat();
  float activeLvl   = data.substring(idx4 + 1, idx5).toFloat();
  float calPerKg    = data.substring(idx5 + 1, idx6).toFloat();
  int feedingCount  = data.substring(idx6 + 1).toInt();

  Serial.println("========== 급식 명령 수신 ==========");
  Serial.print("강아지: "); Serial.println(name);
  Serial.print("체급: "); Serial.println(size);
  Serial.print("체중: "); Serial.print(weight); Serial.println(" kg");
  Serial.print("활동수준: "); Serial.println(activeLvl);
  Serial.print("칼로리(kcal/kg): "); Serial.println(calPerKg);
  Serial.print("급여횟수: "); Serial.println(feedingCount);
  Serial.println("=================================");

  float target = calculatePortionGrams(feedingCount, weight, activeLvl, calPerKg);
  performFeeding(target);
}

// -------------------------------
// 급식량 계산
// -------------------------------
float calculatePortionGrams(int feedingCount, float dogWeight, float activeLvl, float calPerKg) {
  RER = 70 * pow(dogWeight, 0.75);
  DER = RER * activeLvl;
  float dailyGrams = (DER / calPerKg) * 1000.0;
  portionGrams = dailyGrams / feedingCount;

  Serial.println("===== 급식량 계산 =====");
  Serial.print("체중(kg): "); Serial.println(dogWeight);
  Serial.print("활동계수: "); Serial.println(activeLvl);
  Serial.print("급여 횟수: "); Serial.println(feedingCount);
  Serial.print("사료 kcal/kg: "); Serial.println(calPerKg);
  Serial.print("RER: "); Serial.print(RER); Serial.println(" kcal");
  Serial.print("DER: "); Serial.print(DER); Serial.println(" kcal");
  Serial.print("총 하루 사료량: "); Serial.print(dailyGrams); Serial.println(" g");
  Serial.print("👉 1회 사료량: "); Serial.print(portionGrams); Serial.println(" g");
  Serial.println("======================");

  return portionGrams;
}

// -------------------------------
// HX711 안정 무게 측정 (✅ 그릇 무게 자동 보정)
// -------------------------------
float getSuperStableWeight() {
  const int numReadings = 10;
  float readings[numReadings];
  float sum = 0;

  for (int i = 0; i < numReadings; i++) {
    readings[i] = hx711.get_units();
    delay(30);
  }

  for (int i = 0; i < numReadings; i++) sum += readings[i];
  float avg = sum / numReadings;

  float filteredSum = 0;
  int filteredCount = 0;
  for (int i = 0; i < numReadings; i++) {
    if (abs(readings[i] - avg) < 0.02) {
      filteredSum += readings[i];
      filteredCount++;
    }
  }

  float stableWeight = (filteredCount > 0) ? (filteredSum / filteredCount) : avg;

  // ✅ 그릇 무게 보정
  stableWeight -= BOWL_WEIGHT;
  if (stableWeight < 0) stableWeight = 0;

  Serial.print("[WEIGHT] 안정 무게 (보정 후): ");
  Serial.print(stableWeight);
  Serial.println(" g");
  return stableWeight;
}

// -------------------------------
// 서보 1회 동작
// -------------------------------
void runServoOnce() {
  Serial.println("[MOTOR] 서보 동작 시작");
  for (int a = 0; a <= 180; a++) {
    servo.write(a);
    delay(5);
  }
  delay(100);
  for (int a = 180; a >= 0; a--) {
    servo.write(a);
    delay(5);
  }
  Serial.println("[MOTOR] 서보 동작 종료");
}

// -------------------------------
// 목표 사료량까지 반복 급여
// -------------------------------
void performFeeding(float targetGrams) {
  Serial.println("===== 자동 급여 시작 =====");
  feedingActive = true;
  t_motor = 0;  // 초기화

  float currentWeight = getSuperStableWeight();
  float diff = targetGrams - currentWeight;

  // ✅ 이미 목표량 이상일 때 예외처리
  if (diff <= TOLERANCE) {
    Serial.println("⚠️ 이미 목표량 이상이므로 급식 생략");
    feedingActive = false;
    return;
  }

  int cycle = 0;

  while (diff > TOLERANCE) {
    cycle++;

    // ✅ 최초 모터 동작 시각 기록
    if (t_motor == 0) {
      t_motor = millis();
      Serial.print("[METRIC] Motor first start time (t_motor) = ");
      Serial.print(t_motor);
      Serial.println(" ms");
    }

    Serial.print("[CYCLE "); Serial.print(cycle); Serial.println("]");
    runServoOnce();
    delay(1000);

    currentWeight = getSuperStableWeight();
    diff = targetGrams - currentWeight;

    Serial.print("남은 목표량: ");
    Serial.print(diff);
    Serial.println(" g");
  }

  feedingActive = false;
  Serial.println("✅ 목표 사료량 도달 → 급식 완료");
  Serial.println("===========================");
  Serial.println("[DONE]");

}
