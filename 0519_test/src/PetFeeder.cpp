#include "PetFeeder.h"

// 생성자: 한 번만 하드웨어 초기화하기 위해 initFeeder() 추가 호출
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

  // **하드웨어 초기화는 생성자에서 한 번만**

}

// 전체 급식 루틴 실행 함수
void PetFeeder::runFeedingRoutine() {
  feedFood();        // 정량 사료 투입
  soakFood();        // 1차 물 주입 + 불림 (1:1 비율)
  addExtraWater();   // 점도 조절용 물 주입
  grindFood();       // 사료 분쇄
  alertFor3Seconds();// 완료 알림 (LED 등)
  checkFoodLevelAfterGrindDelay();
}

// RER/DER 기반 1회 급식량 계산
void PetFeeder::calculatePortion() {
  calculatePortionGrams(feedCount, feedTimes, petWeight, activityLevel, kcalPerKg);
}

// (헤더에서 feedFood() 시그니처는 그대로 두고)
void PetFeeder::feedFood() {
   
   feedFoodProcess(portionGrams);

}


// 1차 물 주입 + 불림 처리
void PetFeeder::soakFood() {
  soakFoodProcess(baseWaterVolume);
}

// 점도 조절을 위한 추가 물 주입
void PetFeeder::addExtraWater() {
  addExtraWaterProcess(extraWaterVolume);
}

// 모터를 이용한 분쇄 처리
void PetFeeder::grindFood() {
  initmotorGrinder();
  motorGrinder();
  if (isGrindingDone()) {
    grindingDone       = true;
    grindingDoneTime   = millis();    // 분쇄 완료 시각 저장
    foodLevelChecked   = false;
  }
}

void PetFeeder::checkFoodLevelAfterGrindDelay() {
  if (!grindingDone || foodLevelChecked) return;

  unsigned long currentTime = millis();
  if (currentTime - grindingDoneTime >= 1800000) { // 30분
    Serial.println("⏱️ 분쇄 후 30분 경과 → 잔량 측정");

    initFeedingSystem();  
    checkFoodLevel();     
    foodLevelChecked = true;
  }
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