// foodSoaking.cpp

#include "c_foodSoaking.h"
#include "Watercontrol.h"
#include <Arduino.h>
#include "globals.h"


// 5 ml 단위로 반올림해서 runWaterProcess() 호출
static int toUnits5ml(float volumeMl) {
    return int((volumeMl + 4) / 5);
}

void soakFoodProcess(float baseWaterVolume) {
  Serial.print(F("▶ 불림용 물 주입 시작: "));
  Serial.print(baseWaterVolume, 1);
  Serial.println(F(" ml"));

  int units = toUnits5ml(baseWaterVolume);
  runWaterProcess(units);   // 펌프 ON → 목표 도달 시 OFF

  waitSoaking();            // 30분 대기
  Serial.println(F("✅ 불림 완료"));
}