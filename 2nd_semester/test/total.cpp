#include <Servo.h>

// -----------------------------------
// 핀 정의
// -----------------------------------
const int MAIN1_PIN = 10;  // 사료통 1 메인
const int MINI1_PIN = 6;   // 사료통 1 미니
const int MAIN2_PIN = 9;   // 사료통 2 메인
const int MINI2_PIN = 5;   // 사료통 2 미니

Servo main1, mini1, main2, mini2;

// -----------------------------------
// 부드럽게 왕복시키는 함수
// -----------------------------------
void sweep(Servo &s, int maxAngle = 180, int stepMs = 5, int holdMs = 150) {
  for (int a = 0; a <= maxAngle; a++) {
    s.write(a);
    delay(stepMs);
  }
  delay(holdMs);
  for (int a = maxAngle; a >= 0; a--) {
    s.write(a);
    delay(stepMs);
  }
}

// -----------------------------------
// 사료통 세트 구동
// -----------------------------------
void operateFeeder(int feederId) {
  if (feederId == 1) {
    Serial.println("\n[FEEDER 1] start");
    sweep(main1, 165, 5, 100); // 메인
    delay(300);
    sweep(mini1, 180, 5, 150); // 미니
    Serial.println("[FEEDER 1] done");
  }
  else if (feederId == 2) {
    Serial.println("\n[FEEDER 2] start");
    sweep(main2, 165, 5, 100); // 메인
    delay(300);
    sweep(mini2, 180, 5, 150); // 미니
    Serial.println("[FEEDER 2] done");
  }
}

// -----------------------------------
// setup / loop
// -----------------------------------
void setup() {
  Serial.begin(115200);
  Serial.println("[READY] Feeder 1&2 test (no HX711)");
  Serial.println("Type 1 or 2 in Serial Monitor to test each feeder.\n");

  main1.attach(MAIN1_PIN, 500, 2500);
  mini1.attach(MINI1_PIN, 500, 2500);
  main2.attach(MAIN2_PIN, 500, 2500);
  mini2.attach(MINI2_PIN, 500, 2500);

  main1.write(0);
  mini1.write(0);
  main2.write(0);
  mini2.write(0);
  delay(1000);
}

void loop() {
  if (Serial.available()) {
    char c = Serial.read();
    if (c == '1') {
      operateFeeder(1);
    } 
    else if (c == '2') {
      operateFeeder(2);
    }
  }
}
