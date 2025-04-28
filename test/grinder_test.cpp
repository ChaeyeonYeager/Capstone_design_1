#include <Arduino.h>

// 사료분쇄 핀 연결 설정
const int IN1 = 7;
const int IN2 = 6;
const int ENA = 5;  // PWM 핀
const int speakerPin = 11; // 스피커의 S 핀 연결된 디지털 핀

bool isGrinding = false;  // 그라인딩 상태

// ✅ 모터 초기화 함수
void initmotorGrinder() {
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    pinMode(ENA, OUTPUT);
    pinMode(speakerPin, OUTPUT);
}

// ✅ 모터 동작 순서 함수
void motorGrinder() {
  Serial.println("🚗 천천히 회전 시작");
  rotateMotor(100);   // 천천히 회전
  delay(5000);        // 5초 유지

  Serial.println("🚀 최대 속도 회전 시작");
  rotateMotor(255);   // 최대 속도 회전
  delay(10000);       // 10초 유지

  Serial.println("🛑 정지");
  stopMotor();        // 모터 정지
  delay(5000);        // 정지 상태 유지

  alertFor3Seconds(); // 알림 소리 3초간

  isGrinding = true;
}

// ✅ 정방향 회전 함수 (속도 지정)
void rotateMotor(int speed) {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, speed);
}

// ✅ 정지 함수
void stopMotor() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, 0);
}

// ✅ 알림 함수 (3초간 삐 소리)
void alertFor3Seconds() {
  digitalWrite(speakerPin, HIGH); // 삐 소리 시작
  delay(3000);                    // 3초간 유지
  digitalWrite(speakerPin, LOW);  // 소리 끄기
}

bool isGrindingDone() {
    return isGrinding;
}

// setup 함수에서 핀 초기화 및 바로 motorGrinder 호출
void setup() {
    Serial.begin(9600);  // 시리얼 통신 시작
    initmotorGrinder();   // 모터 그라인더 초기화

    // 전원을 켜면 motorGrinder 함수 실행
    motorGrinder();  // 이 함수가 전원을 연결하면 바로 실행됨
}

// loop 함수는 비워두어도 무방 (여기서 계속 실행되는 반복 작업이 없기 때문에)
void loop() {
    // 특별한 작업이 없으면 loop를 비워두거나 필요한 다른 작업을 추가할 수 있음
}
