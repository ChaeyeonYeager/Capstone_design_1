#include <Servo.h>

Servo myServo;      // Servo 객체 생성
const int servoPin = 9;  // 서보모터 제어 핀 (PWM 핀)

void setup() {
  myServo.attach(servoPin);  // 핀에 서보모터 연결
  myServo.write(0);          // 초기 위치를 0°로 설정
}

void loop() {
  // 1) 90°로 회전
  myServo.write(90);
  delay(3000);  // 3초 대기

  // 2) 다시 0°로 복귀
  myServo.write(0);
  delay(3000);  // 3초 대기

  // loop()가 끝나면 다시 처음으로 돌아가 반복
}
