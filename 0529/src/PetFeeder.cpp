// src/PetFeeder.cpp
#include "PetFeeder.h"

// 생성자: 보정 루틴은 main.cpp에서 처리됨
PetFeeder::PetFeeder(String name, float weight, int age,
                     int feedCount, String feedTimes[],
                     float activityLevel, int kcalPerKg,
                     int viscosityLevel)
  : petName(name),
    petWeight(weight),
    petAge(age),
    feedCount(feedCount),
    activityLevel(activityLevel),
    kcalPerKg(kcalPerKg),
    viscosityLevel(viscosityLevel),
    feedingDone(false),
    soakingDone(false),
    grindingDone(false),
    foodLevelChecked(false)
{
  // 급식 시간 복사
  for (int i = 0; i < feedCount; i++) {
    this->feedTimes[i] = feedTimes[i];
  }
  // 1회 급식량 계산
  calculatePortion();
  // 물량 계산
  baseWaterVolume  = portionGrams;
  Serial.print(F("🔍 전달된 portionGrams 값: "));
  Serial.println(baseWaterVolume); 
  float ratio;
  if (viscosityLevel == 1) 
  {
    ratio = 2.5;
  }
  else if (viscosityLevel == 2) 
  {
    ratio = 3.0;
  }
  else 
  {
    ratio = 3.5;
  }

  extraWaterVolume = portionGrams * ratio;

  initFeeder();  // 하드웨어 초기화
}

void PetFeeder::runFeedingRoutine() {
  feedFood();
  // soakFood();
  // ▶ 시연용 대기
  // Serial.println(F("불림 시연이 끝나면 'y'를 입력하세요."));
  // while (true) {
  //   if (Serial.available()) {
  //     char c = Serial.read();
  //     if (c == 'y' || c == 'Y') {
  //       Serial.println(F("시작! 분쇄 단계로 이동합니다."));
  //       break;
  //     }
  //   }
  //   delay(10);  // 너무 빡빡하게 돌지 않도록 약간의 여유
  // }

  // 그 다음 단계로 계속 진행
  // grindFood();
  // checkFoodLevelAfterGrindDelay();
}

// RER/DER 기반 사료량 계산 호출
void PetFeeder::calculatePortion() {
  portionGrams = calculatePortionGrams(feedCount, feedTimes, petWeight, activityLevel, kcalPerKg);
  // calculatePortionGrams 내부에서 전역 변수 portionGrams 세팅
}

// 단계별 처리
void PetFeeder::feedFood() {
  feedTarget = portionGrams;
  feedFoodProcess();
}

void PetFeeder::soakFood() {
  initWaterSystem();
  soakFoodProcess(baseWaterVolume);
}

void PetFeeder::addExtraWater() {
  addExtraWaterProcess(extraWaterVolume);
}

void PetFeeder::grindFood() {
  initmotorGrinder();
  motorGrinder();
  if (isGrindingDone()) {
    grindingDone     = true;
    grindingDoneTime = millis();
    foodLevelChecked = false;
  }
}

void PetFeeder::checkFoodLevelAfterGrindDelay() {
  if (!grindingDone || foodLevelChecked) return;
  if (millis() - grindingDoneTime >= 1800000UL) {
    Serial.println(F("⏱️ 30분 경과, 잔량 측정 시작"));
    initFeedingSystem();
    checkFoodLevel();
    foodLevelChecked = true;
  }
}
