#include <Arduino.h>
#include <Servo.h>

Servo s;
const int SERVO_PIN = 10;   // 메가에서 Servo는 2~13, 44~46 권장

void setup() {
  Serial.begin(115200);
  s.attach(SERVO_PIN);                                           
 
  delay(500);
  Serial.println("시리얼 모니터에 '7'을 입력하면 서보가 한번 동작합니다.");
}

void loop() {
  if (Serial.available() > 0) {
    char c = Serial.read();
    if (c == '7') {
      Serial.println("[RUN] 서보 한번 동작 시작");

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

      Serial.println("== 360도 완료,정지 ==");
      

      
      Serial.println("[STOP] 동작 완료");
    }
  }
}
