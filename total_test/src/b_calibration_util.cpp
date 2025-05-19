#include <EEPROM.h>
#include "globals.h"

#define EEPROM_ADDR 0

void saveCalibrationFactor(float factor) {
  EEPROM.put(EEPROM_ADDR, factor);
}

float loadCalibrationFactor() {
  float f;
  EEPROM.get(EEPROM_ADDR, f);

  // 유효성 검사 (이상한 값 방지)
  if (f < -1000 || f > 1000) {
    return -25.0;  // 기본값 반환
  }

  return f;
}
