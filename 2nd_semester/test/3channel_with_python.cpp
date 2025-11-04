#include <Arduino.h>
#include <Servo.h>
#include <HX711.h>
#include <math.h>

/* ===========================================
 *  단일 급식기 (메인 서보만 동작)
시리얼로 1, 2, 3 입력을 주었을 때, 각각 소형, 중형, 대형견용 사료통의 서보 모터가 동작 함.
 *  - 1: 소형 세트 (무게감지 포함)
 *  - 2: 중형 세트
 *  - 3: 대형 세트
 *  - 'r': 무게감지 테스트 (소형 세트용)
 * 
 * =========================================== */

const int FEEDER_COUNT = 3;
enum FeederType : uint8_t {
  FEEDER_SMALL  = 0,
  FEEDER_MEDIUM = 1,
  FEEDER_LARGE  = 2
};

// 현재 선택된 세트
int currentFeeder = FEEDER_SMALL;

// -------------------------------
// 핀 설정
// -------------------------------
const int SERVO_PIN[FEEDER_COUNT] = {10, 11, 6};

// ✅ 로드셀 (소형 세트 전용)
const uint8_t HX_DT  = 3;
const uint8_t HX_SCK = 2;

Servo servo[FEEDER_COUNT];
HX711 hx711;

// -------------------------------
// 로드셀 보정값
// -------------------------------
const uint32_t CAL_OFFSET = 295745;
const float    CAL_SCALE  = 872.280761;
const float BOWL_WEIGHT_G = 100.0f;
const float TOLERANCE     = 2.0f;
const int   SERVO_STEP_MS = 3;
const int   SETTLE_MS     = 600;

// -------------------------------
// 안정된 무게 측정 (g)
// -------------------------------
float getSuperStableWeight() {
  const int N = 10;
  float r[N], sum = 0;

  for (int i = 0; i < N; i++) {
    r[i] = hx711.get_units();
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
  stable -= BOWL_WEIGHT_G;
  if (stable < 0) stable = 0;

  Serial.print("[WEIGHT] net=");
  Serial.print(stable, 2);
  Serial.println(" g");
  return stable;
}

// -------------------------------
// 메인 서보 동작 (0→165→0)
// -------------------------------
void runServoOnce() {
  Serial.println("[MOTOR] cycle start");
  for (int a = 0; a <= 165; a++) {
    servo[currentFeeder].write(a);
    delay(SERVO_STEP_MS);
  }
  delay(100);
  for (int a = 165; a >= 0; a--) {
    servo[currentFeeder].write(a);
    delay(SERVO_STEP_MS);
  }
  Serial.println("[MOTOR] cycle end");
}

// -------------------------------
// 파싱 및 급여량 계산 코드 예시
// -------------------------------
void handleSerialCSV() {
  if (Serial.available()) {
    String line = Serial.readStringUntil('\n');
    line.trim();
    if (line.length() == 0) return;

    Serial.print("[RECV] "); Serial.println(line);

    // CSV 파싱
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

    // -----------------------------
    // 필드별 파싱
    // -----------------------------
    String name = parts[0];
    float scorePct = parts[1].toFloat();
    String size = parts[2];
    float weight = parts[3].toFloat();      // kg
    float activeLvl = parts[4].toFloat();   // 예: 1.2
    float calPerKg = parts[5].toFloat();    // kcal/kg/day
    int feedingCount = parts[6].toInt();    // 2회/일

    // -----------------------------
    // 체급 → 서보 선택
    // -----------------------------
    if (size == "small") currentFeeder = FEEDER_SMALL;
    else if (size == "medium") currentFeeder = FEEDER_MEDIUM;
    else if (size == "large") currentFeeder = FEEDER_LARGE;
    else currentFeeder = FEEDER_SMALL; // default

    Serial.print("[INFO] Name="); Serial.println(name);
    Serial.print("[INFO] Size="); Serial.println(size);

    // -----------------------------
    // 급여량 계산
    // -----------------------------
    // ① RER (Resting Energy Requirement)
    float RER = 70.0 * pow(weight, 0.75);
    // ② DER (Daily Energy Requirement)
    float DER = RER * activeLvl;
    // ③ 1회 급여량(g)
    //    (칼로리/kg/day * 몸무게) = 하루 총 kcal
    float dailyCal = calPerKg * weight;
    float oneMealCal = dailyCal / feedingCount;
    // 사료 1g당 3.5 kcal 기준 (예시)
    float gramsPerMeal = oneMealCal / 3.5;

    Serial.print("[CALC] RER="); Serial.print(RER,1);
    Serial.print(" DER="); Serial.print(DER,1);
    Serial.print(" kcal/day, 1회="); Serial.print(gramsPerMeal,1);
    Serial.println(" g");

    // -----------------------------
    // 서보 동작 (간단 예시)
    // -----------------------------
    Serial.print("[FEED] "); Serial.print(size);
    Serial.println(" servo 동작 시작");
    runServoOnce();
  }
}


// -------------------------------
// setup / loop
// -------------------------------
void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 2000) {}

  // 메인 서보 초기화
  for (int i = 0; i < FEEDER_COUNT; i++) {
    servo[i].attach(SERVO_PIN[i]);
    delay(200);
    servo[i].write(0);
  }

  // ✅ 로드셀 (소형 세트)
  hx711.begin(HX_DT, HX_SCK);
  hx711.set_offset(CAL_OFFSET);
  hx711.set_scale(CAL_SCALE);

  Serial.println("\n[READY] Single Feeder (Main Servo Only)");
  Serial.println("1: Small (with HX711)");
  Serial.println("2: Medium");
  Serial.println("3: Large");
  Serial.println("r: Read weight (HX711)\n");
}

void loop() {
  handleSerialCSV();  // 새 CSV 명령이 오면 급여 수행
}
