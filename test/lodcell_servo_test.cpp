#include <HX711.h>  // 로드셀(무게측정 센서) 라이브러리
#include <Servo.h>  // 서보모터 제어

HX711 hx711;
Servo myServo;

float calibration_factor = -28000;  // 로드셀 보정값
float tolerancePercent = 5.0;       // 허용 오차 범위 ±5%
bool opened = false;                // 서보모터 열린 상태 여부

#define LOADCELL_DOUT_PIN 2
#define LOADCELL_SCK_PIN 3
#define SERVOPIN 9

#define goal_weight 10.0       // 목표 사료 무게 (g)
#define plate_weight 42.3      // 사료통 무게 (g)

void setup() {
  Serial.begin(115200);
  hx711.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  hx711.set_scale(calibration_factor);
  hx711.tare();  // 초기 영점(0) 설정

  myServo.attach(SERVOPIN);
  myServo.write(0);  // 서보 닫기

  Serial.println("사료통을 올려주세요!");
}

// 이동 평균 기반 정밀 무게 측정 함수
float getSuperStableWeight() {
  const int numReadings = 20;
  float readings[numReadings];
  float sum = 0;

  for (int i = 0; i < numReadings; i++) {
    readings[i] = hx711.get_units();
    delay(30);
  }

  for (int i = 0; i < numReadings; i++) {
    sum += readings[i];
  }
  float average = sum / numReadings;

  float filteredSum = 0;
  int filteredCount = 0;
  for (int i = 0; i < numReadings; i++) {
    if (abs(readings[i] - average) < 0.02) {
      filteredSum += readings[i];
      filteredCount++;
    }
  }

  return (filteredCount > 0) ? (filteredSum / filteredCount) : average;
}

void loop() {
  float weight = getSuperStableWeight();          // kg 단위
  float grossWeight = weight * 1000;              // g 단위 (사료통 포함)
  float netWeight = grossWeight - plate_weight;   // 실제 사료 무게

  Serial.print("Gross (사료통 포함): ");
  Serial.print(grossWeight, 2);
  Serial.print(" g | Net (사료만): ");
  Serial.print(netWeight, 2);
  Serial.println(" g");

  // 서보모터 한 번만 열기 (사료 배출 시작)
  if (!opened && grossWeight > (plate_weight - 5.0)) {
    myServo.write(0);  // 열기
    delay(1000);
    Serial.println("사료 배출 시작!");
    opened = true;
  }

  // 목표량 도달 여부 확인
  if (opened) {
    float minTarget = goal_weight * (1.0 - tolerancePercent / 100.0);
    float maxTarget = goal_weight * (1.0 + tolerancePercent / 100.0);

    if (netWeight >= minTarget && netWeight <= maxTarget) {
      Serial.println("✅ 목표 사료량 도달! 서보모터 닫음.");
      myServo.write(90);  // 닫기
      while (true);      // 동작 종료
    }
  }

  delay(500);  // 측정 주기
}
