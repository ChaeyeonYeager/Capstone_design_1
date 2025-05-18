#include <EEPROM.h>
#include "globals.h"
#include <Arduino.h>    // ✅ Serial, byte 등 아두이노 전용 타입 포함
#include <EEPROM.h>

#define EEPROM_ADDR_CAL_FACTOR 0
#define EEPROM_ADDR_FLAG       sizeof(float)  // 다음 주소

float calibration_factor = -25.0;  // 기본값

void loadCalibrationFromEEPROM() {
  byte flag;
  EEPROM.get(EEPROM_ADDR_FLAG, flag);

  if (flag == 1) {
    float f;
    EEPROM.get(EEPROM_ADDR_CAL_FACTOR, f);
    if (f > -1000 && f < 1000) {
      calibration_factor = f;
      Serial.print("✅ EEPROM에서 보정값 로드: ");
      Serial.println(calibration_factor, 4);
      return;
    }
  }

  Serial.println("⚠️ EEPROM에 보정값 없음 → 기본값 사용 또는 보정 필요");
}

void saveCalibrationToEEPROM(float f) {
  EEPROM.put(EEPROM_ADDR_CAL_FACTOR, f);
  byte flag = 1;
  EEPROM.put(EEPROM_ADDR_FLAG, flag);
  calibration_factor = f;
  Serial.print("💾 EEPROM에 보정값 저장 완료: ");
  Serial.println(f, 4);
}

bool feedDoneToday[MAX_FEEDS] = {false};
bool isFoodInputDone = false;

void resetDailyFeeding() {
    // 모든 급식 완료 플래그를 false로 리셋
    for (int i = 0; i < MAX_FEEDS; i++) {
        feedDoneToday[i] = false;
    }
    isFoodInputDone = false;
}

String getTimeString(const DateTime& dt) {
    char buf[6];
    // 두 자리 시:분 형식
    snprintf(buf, sizeof(buf), "%02d:%02d", dt.hour(), dt.minute());
    return String(buf);
}
