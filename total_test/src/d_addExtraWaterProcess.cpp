// addExtraWaterProcess.cpp

#include "d_addExtraWaterProcess.h"
#include "Watercontrol.h"
#include <Arduino.h>

// 5 ml 단위로 반올림하여 runWaterProcess() 호출
static int toUnits5ml(float volumeMl) {
    return int((volumeMl + 4) / 5);  // ex. 6 ml → (6+4)/5 = 2 units → 10 ml
}

void addExtraWaterProcess(float extraWaterVolume) {
    if (extraWaterVolume > 0.5) {
        Serial.print(F("▶ 추가 물 주입 시작: "));
        Serial.print(extraWaterVolume, 1);
        Serial.println(F(" ml"));

        int units = toUnits5ml(extraWaterVolume);
        runWaterProcess(units);

        Serial.println(F("✅ 추가 물 주입 완료"));
    } else {
        Serial.println(F("ℹ️ 추가 물 투입 불필요"));
    }
}
