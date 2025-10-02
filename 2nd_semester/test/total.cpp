#include <Arduino.h>
#include <Servo.h>
#include <HX711.h>

// 핀 설정
const int SERVO_PIN = 10;
const uint8_t HX_DT = 3;
const uint8_t HX_SCK = 2;

Servo s;
HX711 myScale;

// 하드코딩된 보정값
const uint32_t calibration_offset = 4294672803;
const float    calibration_scale  = -797.160888;

// 측정 주기
const unsigned long READ_DELAY_MS = 500;
bool stop_requested = false;

void setup() {
  Serial.begin(115200);
  while (!Serial) {;}

  s.attach(SERVO_PIN);
  delay(300);

  myScale.begin(HX_DT, HX_SCK);
  myScale.set_offset(calibration_offset);
  myScale.set_scale(calibration_scale);

  Serial.println("\n[START] 자동 측정 시작 (하드코딩 보정값)");
  Serial.println("시리얼에 's' 입력 시 동작 정지");
}

void loop() {
  handle_serial();

  if (!stop_requested) {
    run_servo_once();
    delay(READ_DELAY_MS);
    read_weight();

    run_servo_once();
    delay(READ_DELAY_MS);
    read_weight();
  }
}

// 서보 0 -> 180 -> 0
void run_servo_once() {
  Serial.println("[MOTOR] 서보 동작");
  for (int a = 0; a <= 180; a++) {
    s.write(a);
    delay(5);
  }
  delay(100);
  for (int a = 180; a >= 0; a--) {
    s.write(a);
    delay(5);
  }
}

// 무게 측정 출력
void read_weight() {
  if (myScale.is_ready()) {
    float weight = myScale.get_units(10); // 10회 평균
    Serial.print("[WEIGHT] 측정값: ");
    Serial.print(weight, 2);
    Serial.println(" g");
  } else {
    Serial.println("[ERROR] HX711 not ready");
  }
}

// 시리얼 입력 처리
void handle_serial() {
  if (!Serial.available()) return;

  char c = Serial.read();
  if (c == 's' || c == 'S') {
    stop_requested = true;
    Serial.println("[STOP] 동작 중지됨. 's'로 다시 재시작하려면 코드 재업로드 필요.");
  }
}
