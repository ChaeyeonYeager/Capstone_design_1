#include <Arduino.h>
#include <Servo.h>
#include <HX711.h>
#include <math.h>

/* ===========================================
 *  자동 급식 시스템 (3채널 서보 + 3개 로드셀)
 *  Python → CSV 명령 수신 → 체급별 목표 무게까지 자동 배식
 * =========================================== */

const int FEEDER_COUNT = 3;
enum FeederType : uint8_t {
  FEEDER_SMALL  = 0,
  FEEDER_MEDIUM = 1,
  FEEDER_LARGE  = 2
};

int currentFeeder = FEEDER_SMALL;

// -------------------------------
// 핀 설정
// -------------------------------
const int SERVO_PIN[FEEDER_COUNT] = {10, 11, 6};

// ✅ 각 체급별 로드셀 핀 (DT, SCK)
const uint8_t HX_DT[FEEDER_COUNT]  = {3, 5, 8};
const uint8_t HX_SCK[FEEDER_COUNT] = {2, 4, 7};

Servo servo[FEEDER_COUNT];
HX711 hx711[FEEDER_COUNT];

// -------------------------------
// 로드셀 보정값 (예시)
// -------------------------------
const uint32_t CAL_OFFSET[FEEDER_COUNT] = {67862, -465157, 295209};
const float    CAL_SCALE[FEEDER_COUNT]  = {1103.293579, 1005.908264, 806.155944};
const float BOWL_WEIGHT_G[FEEDER_COUNT] = {109.0, 109.0, 109.0};
const float TOLERANCE = 2.0f;
const int   SERVO_STEP_MS = 3;
const int   SETTLE_MS     = 600;

// -------------------------------
// 안정된 무게 측정 (g)
// -------------------------------
float getSuperStableWeight(int feeder) {
  const int N = 10;
  float r[N], sum = 0;

  for (int i = 0; i < N; i++) {
    r[i] = hx711[feeder].get_units();
    delay(30);
  }

  for (int i = 0; i < N; i++) sum += r[i];
  float avg = sum / N;

  float fsum = 0;
  int cnt = 0;
  for (int i = 0; i < N; i++) {
    if (fabs(r[i] - avg) < 0.02f) {
      fsum += r[i];
      cnt++;
    }
  }
  float stable = (cnt > 0) ? (fsum / cnt) : avg;
  stable -= BOWL_WEIGHT_G[feeder];
  if (stable < 0) stable = 0;

  Serial.print("[WEIGHT] ");
  if (feeder == FEEDER_SMALL) Serial.print("SMALL=");
  else if (feeder == FEEDER_MEDIUM) Serial.print("MEDIUM=");
  else Serial.print("LARGE=");
  Serial.print(stable, 2);
  Serial.println(" g");
  return stable;
}

// -------------------------------
// 메인 서보 동작 (0→165→0)
// -------------------------------
void runServoOnce(int feeder) {
  Serial.print("[MOTOR] cycle start (");
  Serial.print(feeder);
  Serial.println(")");
  for (int a = 0; a <= 165; a++) {
    servo[feeder].write(a);
    delay(SERVO_STEP_MS);
  }
  delay(100);
  for (int a = 165; a >= 0; a--) {
    servo[feeder].write(a);
    delay(SERVO_STEP_MS);
  }
  Serial.println("[MOTOR] cycle end");
}

// -------------------------------
// 목표 급여량까지 자동 급식
// -------------------------------
void feedUntilTarget(int feeder, float target_g) {
  Serial.print("[TARGET] 목표 급여량(");
  Serial.print(feeder);
  Serial.print(") = ");
  Serial.print(target_g, 1);
  Serial.println(" g");

  float current = getSuperStableWeight(feeder);
  int attempt = 0;

  while (current < target_g - TOLERANCE) {
    Serial.print("[FEED] 사이클 "); Serial.println(++attempt);
    runServoOnce(feeder);
    delay(SETTLE_MS);
    current = getSuperStableWeight(feeder);

    if (attempt >= 20) {  // 안전장치
      Serial.println("[WARN] 최대 사이클 초과 → 중단");
      break;
    }
  }

  Serial.print("[DONE] 최종 무게 = ");
  Serial.print(current, 1);
  Serial.println(" g");
}

// -------------------------------
// CSV 수신 + 급여량 계산 + 자동 급식
// -------------------------------
void handleSerialCSV() {
  if (Serial.available()) {
    String line = Serial.readStringUntil('\n');
    line.trim();
    if (line.length() == 0) return;

    Serial.print("[RECV] "); Serial.println(line);

    String parts[8];
    int idx = 0;
    while (line.length() > 0 && idx < 8) {
      int commaIndex = line.indexOf(',');
      if (commaIndex == -1) {
        parts[idx++] = line;
        break;
      } else {
        parts[idx++] = line.substring(0, commaIndex);
        line = line.substring(commaIndex + 1);
      }
    }
    if (idx < 7) {
      Serial.println("[ERR] CSV 필드 개수가 부족합니다");
      return;
    }

    String name = parts[0];
    float scorePct = parts[1].toFloat();
    String size = parts[2];
    float weight = parts[3].toFloat();
    float activeLvl = parts[4].toFloat();
    float calPerKg = parts[5].toFloat();
    int feedingCount = parts[6].toInt();

    // -----------------------------
    // 체급 → 서보/로드셀 선택
    // -----------------------------
    if (size == "small") currentFeeder = FEEDER_SMALL;
    else if (size == "medium") currentFeeder = FEEDER_MEDIUM;
    else if (size == "large") currentFeeder = FEEDER_LARGE;
    else currentFeeder = FEEDER_SMALL;

    Serial.print("[INFO] Name="); Serial.println(name);
    Serial.print("[INFO] Size="); Serial.println(size);

    // -----------------------------
    // 급여량 계산
    // -----------------------------
    float RER = 70.0 * pow(weight, 0.75);
    float DER = RER * activeLvl;
    float dailyCal = calPerKg * weight;
    float oneMealCal = dailyCal / feedingCount;
    float gramsPerMeal = oneMealCal / 3.5;

    Serial.print("[CALC] RER="); Serial.print(RER,1);
    Serial.print(" DER="); Serial.print(DER,1);
    Serial.print(" kcal/day, 1회="); Serial.print(gramsPerMeal,1);
    Serial.println(" g");

    // -----------------------------
    // 목표 무게까지 자동 배식
    // -----------------------------
    feedUntilTarget(currentFeeder, gramsPerMeal);
  }
}

// -------------------------------
// setup / loop
// -------------------------------
void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 2000) {}

  // 서보 초기화
  for (int i = 0; i < FEEDER_COUNT; i++) {
    servo[i].attach(SERVO_PIN[i]);
    delay(200);
    servo[i].write(0);
  }

  // 로드셀 초기화
  for (int i = 0; i < FEEDER_COUNT; i++) {
    hx711[i].begin(HX_DT[i], HX_SCK[i]);
    hx711[i].set_offset(CAL_OFFSET[i]);
    hx711[i].set_scale(CAL_SCALE[i]);
  }

  Serial.println("\n[READY] 3채널 Auto Feeder Ready");
  Serial.println("[INFO] HX711 Pins:");
  Serial.println("  small : DT=3, SCK=2");
  Serial.println("  medium: DT=5, SCK=4");
  Serial.println("  large : DT=8, SCK=7\n");
}

void loop() {
  handleSerialCSV();
}
