// WaterControl.cpp

#include <Arduino.h>
#include "Watercontrol.h"
#include "globals.h"

// 내부 상태
static unsigned int  pulseCount   = 0;
static int           lastState    = HIGH;
static unsigned long lastLogTime  = 0;
static const int     PULSE_PER_5ML = 32;  // 5 ml 당 펄스 수


// 펄스 카운터 (인터럽트 대신 수동 호출)
static void updatePulseCount() {
  int current = digitalRead(flowSensorPin);
  if (current == LOW && lastState == HIGH) {
    pulseCount++;
  }
  lastState = current;
}

// ----------------------------------------------------------------
// @param units5ml “5 ml 단위” 개수, 1 → 5 ml, 2 → 10 ml, …
void runWaterProcess(int units5ml) {
  unsigned int targetPulses = units5ml * PULSE_PER_5ML;
  pulseCount    = 0;
  lastLogTime   = millis();
  isProcessDone = false;

  Serial.print("물 주입 시작: ");
  Serial.print(units5ml * 5);
  Serial.println(" ml");

  digitalWrite(relayPin, HIGH);
  while (pulseCount < targetPulses) {
    updatePulseCount();
    if (millis() - lastLogTime >= 1000) {
      Serial.print("현재 펄스 수: ");
      Serial.println(pulseCount);
      lastLogTime = millis();
    }
  }
  digitalWrite(relayPin, LOW);

  Serial.print("목표 도달 (펄스 ");
  Serial.print(targetPulses);
  Serial.println(") → 물 주입 완료");
  isProcessDone = true;
}

void waitSoaking() {
  delay(1800000);      // 30분 대기
  isProcessDone = true;
}