#include <Arduino.h>
#include <HX711.h>

// =================================================================
// 1. 전역 변수 및 핀 설정
// =================================================================
HX711 myScale;
const uint8_t dataPin = 3;   // HX711의 데이터(DT) 핀
const uint8_t clockPin = 2;  // HX711의 클럭(SCK) 핀

// 캘리브레이션 결과를 저장할 전역 변수 (RAM에 저장됨)
// 전원이 꺼지면 이 값들은 사라지므로, 켤 때마다 새로 구해야 합니다.
uint32_t calibration_offset = 0; 
float calibration_scale = 0.0; 

// =================================================================
// 2. 함수 선언
// =================================================================
void calibrate_and_store();
void print_calibration_result();
void wait_for_enter();
uint32_t read_weight_from_serial();

// =================================================================
// 3. Setup: 초기 설정 및 캘리브레이션 실행
// =================================================================
void setup() {
  Serial.begin(115200);
  Serial.println("\n--- HX711 Dynamic Calibration Scale Initializing ---");
  Serial.print("LIBRARY VERSION: ");
  Serial.println(HX711_LIB_VERSION);
  
  // HX711 초기화
  myScale.begin(dataPin, clockPin);
  
  // 캘리브레이션 함수 실행 및 결과 저장
  calibrate_and_store();

  // 캘리브레이션 결과를 시리얼 모니터에 출력 (확인용)
  print_calibration_result();
}

// =================================================================
// 4. Loop: 실제 무게 측정 (저장된 값을 사용)
// =================================================================
void loop() {
  if (myScale.is_ready()) {
    // 캘리브레이션 값을 사용하여 무게 측정
    float units = myScale.get_units(10); // 10회 측정 평균 무게
    Serial.print("Weight: ");
    Serial.print(units, 2); // 소수점 둘째 자리까지 출력
    Serial.println(" g");
  } else {
    Serial.println("HX711 not found or not ready.");
  }
  delay(500);
}

// =================================================================
// 5. 캘리브레이션 및 보조 함수 구현
// =================================================================

/**
 * @brief 시리얼 모니터 상호작용을 통해 캘리브레이션을 수행하고 결과를 전역 변수에 저장합니다.
 */
void calibrate_and_store() {
  Serial.println("\n\nCALIBRATION START\n=================");
  
  // 1. 영점(Zero Offset) 설정
  Serial.println("1. remove all weight from the loadcell");
  Serial.println("-> press enter to determine zero offset...\n");
  wait_for_enter(); // Enter 대기
  
  Serial.println("-> Determining zero weight offset...");
  myScale.tare(20);
  calibration_offset = myScale.get_offset(); // 전역 변수에 저장
  
  Serial.print("OFFSET calculated: "); 
  Serial.println(calibration_offset);

  // 2. Scale Factor 설정
  Serial.println("\n2. place a KNOWN weight on the loadcell (e.g., 100g)");
  Serial.println("-> enter the weight in (whole) grams and press enter\n");
  
  uint32_t weight = read_weight_from_serial(); // 시리얼 입력 읽기
  Serial.print("WEIGHT entered: "); 
  Serial.println(weight);
  
  // Scale Factor 계산 및 설정
  myScale.calibrate_scale(weight, 20); 
  calibration_scale = myScale.get_scale(); // 전역 변수에 저장
  
  Serial.print("SCALE factor calculated: "); 
  Serial.println(calibration_scale, 6);
  
  // myScale 객체에 최종 캘리브레이션 값 적용 (loop에서 사용하기 위해)
  myScale.set_offset(calibration_offset);
  myScale.set_scale(calibration_scale);
  
  Serial.println("\nCALIBRATION COMPLETE\n");
}

/**
 * @brief 전역 변수에 저장된 최종 캘리브레이션 값을 출력합니다.
 */
void print_calibration_result() {
  Serial.println("==================================================");
  Serial.println("✅ FINAL CALIBRATION VALUES (Stored in RAM):");
  Serial.print("   OFFSET: ");
  Serial.println(calibration_offset);
  Serial.print("   SCALE:  ");
  Serial.println(calibration_scale, 6);
  Serial.println("   --> These values will be lost when power is off.");
  Serial.println("==================================================");
}

/**
 * @brief 시리얼 입력을 비우고 Enter 키 입력(줄 바꿈 문자)을 기다립니다.
 */
void wait_for_enter() {
  while (Serial.available()) Serial.read(); // 시리얼 입력 비우기
  while (Serial.available() == 0);          // Enter 대기
  while (Serial.available()) Serial.read(); // Enter 후 남은 문자 비우기
}

/**
 * @brief 시리얼로부터 정수형 무게 값을 읽어 반환합니다.
 */
uint32_t read_weight_from_serial() {
  uint32_t weight = 0;
  // Enter 키('\n')가 입력될 때까지 숫자를 읽어 weight 변수에 저장
  while (Serial.peek() != '\n') {
    if (Serial.available()) {
      char ch = Serial.read();
      if (isdigit(ch)) {
        weight *= 10;
        weight = weight + (ch - '0');
      }
    }
  }
  return weight;
}