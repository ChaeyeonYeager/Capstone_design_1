#include <Arduino.h>
#include <Servo.h>
#include <HX711.h>
#include <math.h>

/* ===========================================
 *  단일 급식기 (Python 트리거 → t_motor 전송)
 *  - '1', '2', '3' 수신 시 대응 서보 모터 동작
 *  - 모터 시작 직전 시점에 [MOTOR] cycle start 전송
 *  - (Python이 이걸 받아서 t_motor로 기록)
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

// ✅ 로드셀 (소형 세트)
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

// -------------------------------
// 안정된 무게 측정
// -------------------------------
// float getSuperStableWeight() {
//   const int N = 10;
//   float r[N], sum = 0;
//   for (int i = 0; i < N; i++) {
//     r[i] = hx711.get_units();
//     delay(30);
//   }
//   for (int i = 0; i < N; i++) sum += r[i];
//   float avg = sum / N;
//   float fsum = 0;
//   int cnt = 0;
//   for (int i = 0; i < N; i++) {
//     if (fabs(r[i] - avg) < 0.02f) {
//       fsum += r[i];
//       cnt++;
//     }
//   }
//   float stable = (cnt > 0) ? (fsum / cnt) : avg;
//   stable -= BOWL_WEIGHT_G;
//   if (stable < 0) stable = 0;

//   Serial.print("[WEIGHT] net=");
//   Serial.print(stable, 2);
//   Serial.println(" g");
//   return stable;
// }

// -------------------------------
// 메인 서보 동작
// -------------------------------
void runServoOnce() {
  // ✅ Python이 이 시점을 t_motor로 기록함
  Serial.println("[MOTOR] cycle start");

  for (int a = 0; a <= 165; a++) {
    servo[feeder].write(a);
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
  // 데이터가 없으면 리턴
  if (!Serial.available()) return;

  // '\n' 기준으로 한 줄 읽기 (타임아웃 내에서 대기)
  String cmd = Serial.readStringUntil('\n');
  cmd.trim();  // 공백, \r 제거

  // 아무 데이터도 없으면 리턴
  if (cmd.length() == 0) return;

  // 잘못된 입력 대비
  if (cmd != "1" && cmd != "2" && cmd != "3" && cmd != "r" && cmd != "R") {
    Serial.print("[WARN] Unknown command: ");
    Serial.println(cmd);
    return;
  }

  // ---------- 실제 명령 처리 ----------
  if (cmd == "1") {
    currentFeeder = FEEDER_SMALL;
    Serial.println("[SELECT] SMALL FEEDER");
    runServoOnce();
  } 
  else if (cmd == "2") {
    currentFeeder = FEEDER_MEDIUM;
    Serial.println("[SELECT] MEDIUM FEEDER");
    runServoOnce();
  } 
  else if (cmd == "3") {
    currentFeeder = FEEDER_LARGE;
    Serial.println("[SELECT] LARGE FEEDER");
    runServoOnce();
  } 
  else if (cmd == "r" || cmd == "R") {
    Serial.println("[TEST] HX711 weight reading");
    getSuperStableWeight();
  }
}

// -------------------------------
// setup / loop
// -------------------------------
void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 2000) {}

  for (int i = 0; i < FEEDER_COUNT; i++) {
    servo[i].attach(SERVO_PIN[i]);
    delay(200);
    servo[i].write(0);
  }

  hx711.begin(HX_DT, HX_SCK);
  hx711.set_offset(CAL_OFFSET);
  hx711.set_scale(CAL_SCALE);

  Serial.println("\n[READY] Feeder system ready.");
  Serial.println("Commands: 1=small, 2=medium, 3=large, r=read weight\n");
}

void loop() {
  handleSerial();
}
