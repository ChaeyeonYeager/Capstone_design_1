// 임의의 사료량을 설정하고 해당 사료량에 도달하면 서보모터로 입구를 닫히는 부분 코드

#include <HX711.h>      // 로드셀 라이브러리
#include <Servo.h>      // 서보모터 라이브러리

// ======= 핀 설정 =======
#define DOUT_PIN A1      // HX711 DOUT
#define CLK_PIN  A0      // HX711 CLK
#define SERVO_PIN 9      // 서보모터 제어 핀

// ======= 급식 기준 무게(g) =======
#define TARGET_WEIGHT 30.0       // 목표 사료 g
#define TOLERANCE 0.05           // ±5% 허용 오차

HX711 scale;
Servo servo;

void setup() {
  Serial.begin(9600);

  scale.begin(DOUT_PIN, CLK_PIN);
  scale.set_scale(2280.f); // 보정 필요
  scale.tare();            // 초기 무게 설정

  servo.attach(SERVO_PIN);
  servo.write(0); // 문 닫힌 상태로 시작

  Serial.println("🚀 로드셀 + 서보모터 테스트 시작!");
}

void loop() {
  startFeeding();  // loop 안에서는 이 함수만 호출
}

// ======= 급식 처리 함수 =======
void startFeeding() {
  if (Serial.available()) {
    Serial.read(); // 입력 버퍼 비우기
    Serial.println("급식 시작! 문 열림");
    servo.write(90); // 문 열기
    delay(3000);     // 사료 떨어질 시간

    float minWeight = TARGET_WEIGHT * (1.0 - TOLERANCE);
    float maxWeight = TARGET_WEIGHT * (1.0 + TOLERANCE);
    float measured = 0;

    // 로드셀 감시: 목표 무게 도달하면 문 닫기
    while (true) {
      measured = scale.get_units();
      Serial.println("현재 무게: " + String(measured, 1) + "g");
      if (measured >= minWeight && measured <= maxWeight) {
        break;
      }
      delay(200);
    }

    servo.write(0); // 문 닫기
    Serial.println("목표 무게 도달 → 문 닫힘");
  }

  delay(500);
}

