#include <Arduino.h>
#include <Servo.h>
#include <HX711.h>

// ========================== 핀/객체 ==========================
Servo s;
const int SERVO_PIN = 10;     // Mega: 2~13, 44~46 권장

HX711 myScale;
const uint8_t HX_DT = 3;      // HX711 DT
const uint8_t HX_SCK = 2;     // HX711 SCK

// ========================== 캘리브레이션 저장 값 ==========================
// 전원이 꺼지면 사라짐(RAM). 영구 저장하려면 EEPROM 사용 권장.
uint32_t calibration_offset = 0; 
float    calibration_scale  = 0.0f;

// ========================== 상태/주기 ==========================
bool show_readings = true;       // 실시간 계측 출력 on/off
unsigned long last_read_ms = 0;  // 비차단 계측 타이밍
const unsigned long READ_PERIOD_MS = 500;

// ========================== 선언부 ==========================
void print_menu();
void handle_serial();
void run_servo_once();
void calibrate_and_store();
void print_calibration_result();
void wait_for_enter();
uint32_t read_weight_from_serial();

// ========================== Setup ==========================
void setup() {
  Serial.begin(115200);
  while (!Serial) {;}  // USB 안정화

  // 서보
  s.attach(SERVO_PIN);
  delay(300);

  // HX711
  myScale.begin(HX_DT, HX_SCK);

#if defined(HX711_LIB_VERSION)
  Serial.print("HX711 LIB VERSION: ");
  Serial.println(HX711_LIB_VERSION);
#endif

  Serial.println("\n--- Combined Servo + HX711 ---");
  print_menu();

  // 초기에는 캘리브레이션 값을 0/0.0으로 두고, 필요 시 'c'로 교정
  // 이전에 얻은 값이 있다면 아래처럼 직접 세팅 가능:
  // calibration_offset = ...;
  // calibration_scale  = ...;
  if (calibration_scale != 0.0f) {
    myScale.set_offset(calibration_offset);
    myScale.set_scale(calibration_scale);
  }
}

// ========================== Loop ==========================
void loop() {
  handle_serial();

  // 비차단 주기 계측
  if (show_readings && (millis() - last_read_ms >= READ_PERIOD_MS)) {
    last_read_ms = millis();
    if (myScale.is_ready()) {
      // 캘리브레이션 값이 설정되어 있지 않으면 0으로 나올 수 있음
      float units = myScale.get_units(10); // 10회 평균
      Serial.print("Weight: ");
      Serial.print(units, 2);
      Serial.println(" g");
    } else {
      Serial.println("HX711 not ready.");
    }
  }
}

// ========================== 시리얼 명령 처리 ==========================
void handle_serial() {
  if (!Serial.available()) return;

  char c = Serial.read();
  // CR/LF 처리: 줄바꿈 문자는 무시
  if (c == '\r' || c == '\n') return;

  switch (c) {
    case '7':
      Serial.println("[RUN] 서보 한번 동작 시작");
      run_servo_once();
      Serial.println("== 360도 완료, 정지 ==");
      Serial.println("[STOP] 동작 완료");
      break;

    case 'c':
      Serial.println("\n[CAL] 캘리브레이션 시작");
      {
        bool prev = show_readings;
        show_readings = false;   // 교정 중 출력 중단
        calibrate_and_store();
        print_calibration_result();
        show_readings = prev;    // 원래 상태 복귀
      }
      break;

    case 'p':
      print_calibration_result();
      break;

    case 't':
      show_readings = !show_readings;
      Serial.print("[TOGGLE] 실시간 계측 출력: ");
      Serial.println(show_readings ? "ON" : "OFF");
      break;

    case 'h':
    case '?':
      print_menu();
      break;

    default:
      Serial.print("[INFO] 알 수 없는 명령: ");
      Serial.println(c);
      print_menu();
      break;
  }
}

// ========================== 기능 구현 ==========================
void run_servo_once() {
  // 0 -> 180
  for (int a = 0; a <= 180; a++) {
    s.write(a);
    delay(5);
  }
  delay(100);
  // 180 -> 0
  for (int a = 180; a >= 0; a--) {
    s.write(a);
    delay(5);
  }
}

void calibrate_and_store() {
  Serial.println("\nCALIBRATION START");
  Serial.println("-----------------");

  // 1) 영점 설정
  Serial.println("1) 모든 하중 제거 후 Enter를 누르세요.");
  wait_for_enter();

  Serial.println("-> Zero offset 측정중...");
  myScale.tare(20); // 20회 평균으로 영점
#if defined(HX711_LIB_VERSION)
  calibration_offset = myScale.get_offset();  // 일부 라이브러리에 존재
#else
  // 사용 중인 라이브러리에 get_offset()이 없다면, tare() 이후 내부 상태를 따로 저장할 수 없음.
  // 이 경우엔 아래처럼 read_average() 등으로 추정하거나 offset 사용을 생략하세요.
  calibration_offset = 0;
#endif
  Serial.print("OFFSET: ");
  Serial.println(calibration_offset);

  // 2) 스케일 팩터 설정
  Serial.println("\n2) 기준 추(예: 100g)를 올린 뒤, 정확한 무게를 g 단위 정수로 입력하고 Enter.");
  uint32_t weight = read_weight_from_serial();
  Serial.print("입력 무게: ");
  Serial.print(weight);
  Serial.println(" g");

  // 일부 HX711 라이브러리는 calibrate_scale()가 없음. 있다면 사용, 없으면 수동 계산 필요.
#if defined(HX711_LIB_VERSION)
  myScale.calibrate_scale(weight, 20);     // 20회 평균 기반 보정
  calibration_scale = myScale.get_scale(); // 스케일 읽기
#else
  // 보그데(bogde) 라이브러리류에는 calibrate_scale() 없음.
  // 간단 대안(권장 X, 참고용): 현재 읽기값과 실제 무게 비로 scale 역산
  {
    long raw = myScale.read_average(20);
    // 아주 단순한 예: scale = (raw - offset) / 실제무게
    // 실제로는 tare/offset 처리 등 라이브러리별 차이 고려 필요.
    float scale_est = (float)raw / (float)weight;
    myScale.set_scale(scale_est);
    calibration_scale = scale_est;
  }
#endif

  // 최종 값 적용
  myScale.set_offset(calibration_offset);
  myScale.set_scale(calibration_scale);

  Serial.println("\nCALIBRATION COMPLETE");
}

void print_calibration_result() {
  Serial.println("==========================================");
  Serial.println("FINAL CALIBRATION VALUES (RAM):");
  Serial.print("  OFFSET: "); Serial.println(calibration_offset);
  Serial.print("  SCALE : "); Serial.println(calibration_scale, 6);
  Serial.println("  -> 전원 OFF 시 사라짐. 필요시 EEPROM 사용 권장.");
  Serial.println("==========================================");
}

void wait_for_enter() {
  // 입력 버퍼 비우기
  while (Serial.available()) Serial.read();
  // 줄바꿈 도달까지 대기
  while (true) {
    if (Serial.available()) {
      char ch = Serial.read();
      if (ch == '\n' || ch == '\r') break;
    }
  }
}

uint32_t read_weight_from_serial() {
  uint32_t weight = 0;
  // 줄바꿈(CR/LF) 만날 때까지 숫자만 누적
  while (true) {
    while (!Serial.available()) { /* 대기 */ }
    char ch = Serial.read();
    if (ch == '\n' || ch == '\r') break;
    if (isDigit(ch)) {
      weight = weight * 10 + (ch - '0');
    }
    // 숫자 외 문자는 무시
  }
  return weight;
}

void print_menu() {
  Serial.println("\n[명령 안내]");
  Serial.println("  '7' : 서보 한번 동작(0->180->0)");
  Serial.println("  'c' : HX711 캘리브레이션 수행");
  Serial.println("  'p' : 캘리브레이션 결과 출력");
  Serial.println("  't' : 실시간 계측 출력 토글(ON/OFF)");
  Serial.println("  'h' or '?' : 도움말 표시");
}
