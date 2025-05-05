#include "PetFeeder.h"

PetFeeder::PetFeeder(String name, float weight, int age, int feedCount, String feedTimes[],
                     float activityLevel, int kcalPerKg, int viscosityLevel)
    : petName(name), petWeight(weight), petAge(age), feedCount(feedCount),
      activityLevel(activityLevel), kcalPerKg(kcalPerKg), viscosityLevel(viscosityLevel) {

  // 초기 상태 설정
  feedingDone = soakingDone = grindingDone = false;

  // 급식 시간 복사
  for (int i = 0; i < feedCount; i++) {
    this->feedTimes[i] = feedTimes[i];
  }

  // 자동 급식량 계산 (g 단위)
  calculatePortion();

  // 1:1 불림용 물량
  baseWaterVolume = portionGrams;

  // 점도 비율 계산 → 불림용 물 제외한 추가 물량
  float ratio = 1.0;
  if (viscosityLevel == 1) ratio = 1.5;
  else if (viscosityLevel == 2) ratio = 2.0;
  extraWaterVolume = portionGrams * (ratio - 1.0);
}

// 전체 급식 루틴 실행 함수
void PetFeeder::runFeedingRoutine() {
  feedFood();        // 정량 사료 투입
  soakFood();        // 1차 물 주입 + 불림 (1:1 비율)
  addExtraWater();   // 점도 조절용 물 주입
  grindFood();       // 사료 분쇄
  alertFor3Seconds(); // 완료 알림 (LED 등)
}

// RER/DER 기반 1회 급식량 계산
void PetFeeder::calculatePortion() {
  calculatePortionGrams(feedCount, feedTimes, petWeight, activityLevel, kcalPerKg);
}

// 서보모터 + 로드셀 기반 사료 투입
void PetFeeder::feedFood() {
  runFeedingSchedule();
  if (isFeedingDone()) feedingDone = true;
}

// 1차 물 주입 + 불림 처리
void PetFeeder::soakFood() {
  setTargetVolume(baseWaterVolume);  // 1:1 기준
  initPump();

  while (isPumpOn()) {
    pumpUpdate();                    // 목표량까지 주입
  }

  resetVolume();
  Serial.println("불림 완료 (1:1 물 주입)");
  isSoaking = true;
}

// 점도 조절을 위한 추가 물 주입
void PetFeeder::addExtraWater() {
  if (extraWaterVolume > 0) {
    Serial.println("점도 조절용 물 추가 주입 시작");
    setTargetVolume(extraWaterVolume);
    initPump();

    while (isPumpOn()) {
      pumpUpdate();
    }

    resetVolume();
    Serial.println("점도 조절 물 주입 완료");
  } else {
    Serial.println("점도 조절 불필요 (추가 물 없음)");
  }
}

// 모터를 이용한 분쇄 처리
void PetFeeder::grindFood() {
  initmotorGrinder();
  motorGrinder();
  if (isGrindingDone()) grindingDone = true;
}

// 급식 정보 반환 함수들
int PetFeeder::getFeedCount() const {
  return feedCount;
}

String PetFeeder::getFeedTime(int index) const {
  return feedTimes[index];
}

// 각 처리 완료 상태 반환
bool PetFeeder::isFeedingComplete() const {
  return feedingDone;
}

bool PetFeeder::isSoakingComplete() const {
  return soakingDone;
}

bool PetFeeder::isGrindingComplete() const {
  return grindingDone;
}
