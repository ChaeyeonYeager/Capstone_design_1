// src/main.cpp

#include <Arduino.h>
#include <EEPROM.h>
#include "PetFeeder.h"
#include "calibration_routine.h"
#include "globals.h"

const int EEPROM_ADDR = 0;  // EEPROM address for calibration_factor
PetFeeder* demoPet;

// ─── helper function declarations ─────────────────────────────────────────────
void requestString(const char* prompt, String &out);
void requestFloat (const char* prompt, float  &out);
void requestInt   (const char* prompt, int    &out);
void requestFeedTimes(int count, String times[]);
void setupDemo();
// ─────────────────────────────────────────────────────────────────────────────

void setup() {
  Serial.begin(9600);
  while (!Serial);
  // 1) Hardware initialization
  initFeeder();

  // 2) Load calibration factor from EEPROM
  // 2) Load calibration factor from EEPROM
float storedFactor;
EEPROM.get(EEPROM_ADDR, storedFactor);

// 이제 음수도 허용, 0.0f인 경우에만 새 보정
if (storedFactor == 0.0f) {
  Serial.println(F(">> No calibration factor in EEPROM → starting calibration"));
  calibrateZero();
  calibrateContainer();
  EEPROM.put(EEPROM_ADDR, calibration_factor);
  Serial.print(F("💾 Saved calibration factor: "));
} else {
  // 0이 아니면 음수든 양수든 그대로 사용
  calibration_factor = storedFactor;
  Serial.print(F("💾 Loaded calibration factor: "));
  Serial.println(calibration_factor, 4);

  Serial.println(F("Press 'o' to use this factor, or 'x' to recalibrate."));
  while (!Serial.available()) {}
  char c = Serial.read();
  while (Serial.available()) Serial.read();

  if (c=='x' || c=='X') {
    Serial.println(F("✖ Recalibrating..."));
    calibrateZero();
    calibrateContainer();
    EEPROM.put(EEPROM_ADDR, calibration_factor);
    Serial.print(F("💾 New calibration factor saved: "));
    Serial.println(calibration_factor, 4);
  } else {
    Serial.println(F("✔ Using loaded calibration factor."));
  }
}
setupDemo();
}

void loop() {
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    if (cmd.equalsIgnoreCase("run")) {
      Serial.println(F("\n⏳ Starting feeding routine..."));
      demoPet->runFeedingRoutine();
      Serial.println(F("✅ Routine completed. Type 'run' to repeat.\n"));
    }
  }
}

// ─── helper function definitions ────────────────────────────────────────────

void requestString(const char* prompt, String &out) {
  Serial.println(prompt);
  while (!Serial.available()) {}
  out = Serial.readStringUntil('\n');
  out.trim();
  Serial.print(F("→ ")); Serial.println(out);
}

void requestFloat(const char* prompt, float &out) {
  Serial.println(prompt);
  while (!Serial.available()) {}
  out = Serial.parseFloat();
  Serial.readStringUntil('\n');
  Serial.print(F("→ ")); Serial.println(out);
}

void requestInt(const char* prompt, int &out) {
  Serial.println(prompt);
  while (!Serial.available()) {}
  out = Serial.parseInt();
  Serial.readStringUntil('\n');
  Serial.print(F("→ ")); Serial.println(out);
}

void requestFeedTimes(int count, String times[]) {
  for (int i = 0; i < count; i++) {
    char buf[64];
    snprintf(buf, sizeof(buf), "%d) Enter feed time #%d (HH:MM):", 5 + i, i+1);
    requestString(buf, times[i]);
  }
}


void setupDemo()
{
  Serial.println(F("=== Automatic Pet Feeder Demo Setup ==="));
  String name;
  float weight;
  int age, feedCount;
  float activityLevel;
  int kcalPer100g, viscosityLevel;

  requestString("1) Enter pet name:",           name);
  requestFloat ("2) Enter pet weight (kg):",    weight);
  requestInt   ("3) Enter pet age (years):",    age);
  requestInt   ("4) Enter number of feedings per day:", feedCount);

  String feedTimes[MAX];
  requestFeedTimes(feedCount, feedTimes);

  requestFloat ("5) Enter activity level (e.g., 1.2):",       activityLevel);
  requestInt   ("6) Enter kcal per 100g of feed (e.g., 350):", kcalPer100g);
  requestInt   ("7) Enter viscosity level (0:1:2, 1:2.5, 2:1:3):", viscosityLevel);

  demoPet = new PetFeeder(
    name, weight, age,
    feedCount, feedTimes,
    activityLevel, kcalPer100g, viscosityLevel
  );

  Serial.println(F("\nSetup complete!"));
  Serial.println(F("Type 'run' and press Enter → start feeding routine\n"));
}