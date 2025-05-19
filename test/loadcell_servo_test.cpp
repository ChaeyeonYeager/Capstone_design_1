#include "HX711.h"
#include <Servo.h>

#define DT_PIN 2
#define SCK_PIN 3
#define SERVO_PIN 9

HX711 scale;
Servo servo;

bool monitoring = false;
float initialWeight = 0.0;
const float threshold = 20.0;  // g 감소 기준

void setup() {
  Serial.begin(9600);
  scale.begin(DT_PIN, SCK_PIN);
  servo.attach(SERVO_PIN);

  Serial.println("로드셀 초기화 중...");
  scale.set_scale(-25);  // 측정한 보정값
  delay(500);

  Serial.println("0점 보정 중입니다. 로드셀 위에 아무것도 올리지 마세요...");
  delay(3000);
  scale.tare();

  servo.write(90);  // 서보 열림 상태
  Serial.println("보정 완료. 'c' 입력 시 실시간 무게 측정 시작");
}

void loop() {
  if (Serial.available() > 0) {
    char input = Serial.read();

    if (input == 'c' || input == 'C') {
      initialWeight = scale.get_units(10);
      Serial.print("측정 시작. 초기 무게: ");
      Serial.print(initialWeight, 2);
      Serial.println(" g");

      monitoring = true;
    }
  }

  if (monitoring) {
    float currentWeight = scale.get_units(10);
    float diff = initialWeight - currentWeight;

    Serial.print("현재 무게: ");
    Serial.print(currentWeight, 2);
    Serial.print(" g (감소량: ");
    Serial.print(diff, 2);
    Serial.println(" g)");

    if (diff >= threshold) {
      Serial.println("▶ 무게 감소 감지됨! 서보모터 닫기 동작 실행 중...");
      servo.write(0);  // 닫기
      monitoring = false;  // 한 번만 실행
    }

    delay(1000);  // 1초 간격 측정
  }
}
