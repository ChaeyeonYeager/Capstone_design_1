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
// 시리얼 입력 처리
// -------------------------------
void handleSerial() {
  if (Serial.available()) {
    char c = Serial.read();

    // 버퍼 정리
    while (Serial.available()) Serial.read();

    if (c == '1') {
      currentFeeder = FEEDER_SMALL;
      Serial.println("[SELECT] SMALL FEEDER");
      runServoOnce();

      Serial.println("[SMALL] Measuring weight...");
      float weight = getSuperStableWeight();
      Serial.print("[RESULT] ");
      Serial.print(weight);
      Serial.println(" g\n");
    }
    else if (c == '2') {
      currentFeeder = FEEDER_MEDIUM;
      Serial.println("[SELECT] MEDIUM FEEDER");
      runServoOnce();
    }
    else if (c == '3') {
      currentFeeder = FEEDER_LARGE;
      Serial.println("[SELECT] LARGE FEEDER");
      runServoOnce();
    }
    else if (c == 'r' || c == 'R') {
      Serial.println("[TEST] HX711 weight reading");
      float w = getSuperStableWeight();
      Serial.print("[CURRENT WEIGHT] ");
      Serial.print(w);
      Serial.println(" g");
    }
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
  handleSerial();
}
