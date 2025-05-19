#include <Arduino.h>
#include <Pinchanginterrupt.h>

/*
  워터펌프는 100mL의 물을 주입하는 것이 목표입니다.
  유량 센서는 100mL 기준으로 약 98펄스를 발생시키며, 이 값을 목표 펄스로 설정합니다.

  모터는 L298N 드라이버를 통해 제어되며,
  ENA 핀에 PWM 신호를 주어 전압을 조절합니다.
  (PWM 191 ≒ 9V, PWM 255 ≒ 12V 기준)

  펌프는 5초간 동작하고, 이후 5초간 정지합니다.
  펌프가 동작 중일 때 유량 센서의 펄스를 카운트하고,
  누적 펄스 수가 목표(98)에 도달하면 펌프를 정지시킵니다.
*/

// 펌프 제어 핀
const int IN1 = 8;
const int IN2 = 9;
const int ENA = 10;

// 유량 센서 핀
const int flowSensorPin = 2;
volatile int pulseCount = 0;

const int targetPulse = 98; // 100mL 목표 펄스 수 (YM-401A 기준)

bool pumpRunning = false;

// 펄스 카운트 증가 함수
void countPulse() {
  pulseCount++;
}

void setup() {
  // 핀 설정
  pinMode(IN1, OUTPUT); // IN1 HIGH , IN2 LOW → 정방향 전류가 흘러야 함(펌프 동작)
  pinMode(IN2, OUTPUT); // IN1 LOW , IN2 LOW → 정지 상태(펌프 정지)
  pinMode(ENA, OUTPUT); // 아두이노로부터 PWM 신호를 받아 모터 드라이버에서 전압을 조절케 함
  pinMode(flowSensorPin, INPUT_PULLUP); // 유량센서 핀은 풀업 저항을 사용하여 HIGH 상태로 유지

  // 유량센서 인터럽트 설정(LOW,CHANGE대신에 RISING을 사용하는 이유는 유량이 지나가면서 H->L->H로 바뀌는 RISING에만 인터럽트
  // 처리하여 노이즈를 줄이기 위함.)
  attachInterrupt(digitalPinToInterrupt(flowSensorPin), countPulse, RISING);

  // 시리얼 시작
  Serial.begin(9600);
  Serial.println("=== 100mL 정량 주입 시스템 시작 ===");
}


// 펄스를 측정하고 5초후에 다시 100ml 주입을 시작하는 루프
void loop() {
  // 1회만 실행 (펌프 동작)
  if (!pumpRunning) {
    pulseCount = 0;         // 펄스 카운트 초기화
    pumpRunning = true;     // 펌프 동작 시작

    // 펌프 ON
    digitalWrite(IN1, HIGH);  // IN1 HIGH , IN2 LOW → 정방향 전류가 흘러야 함(펌프 동작)
    digitalWrite(IN2, LOW);
    analogWrite(ENA, 191);  // pwm 신호를 191로 설정하여 모터 속도 조절 (9V 기준)
    Serial.println("[펌프 ON] 100mL 목표로 주입 시작...");

    // 목표 도달 여부 체크를 동작 블록 안에서 바로 수행
    while (pumpRunning) {
      if (pulseCount >= targetPulse) {  // 목표 펄스(98) 도달 시 펌프 정지지

        analogWrite(ENA, 0);  // pwm 신호를 0으로 설정하여 모터 정지
        digitalWrite(IN1, LOW); // IN1 LOW , IN2 LOW → 정지 상태(펌프 정지)
        digitalWrite(IN2, LOW);
        pumpRunning = false;

        Serial.println("[펌프 OFF] 목표 100mL 도달!");
        Serial.print("최종 펄스 수: ");
        Serial.println(pulseCount);
        float ml = pulseCount * (100.0 / targetPulse);
        Serial.print("계산된 주입량 (mL): ");
        Serial.println(ml, 1);

        Serial.println("5초간 대기 후 재시작 가능...\n");
        delay(5000);
      }
    }
  }
}


