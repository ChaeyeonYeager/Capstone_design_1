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
  initFeeder();
  initLiquidFeedDetection("SSID", "PASSWORD", "http://<서버_IP>:5000/analyze");
}

// 전체 급식 루틴 실행 함수
void PetFeeder::runFeedingRoutine() {
  feedFood();        // 정량 사료 투입
  soakFood();        // 1차 물 주입 + 불림 (1:1 비율)
  addExtraWater();   // 점도 조절용 물 주입
  grindFood();       // 사료 분쇄
  alertFor3Seconds();// 완료 알림 (LED 등)
  checkLiquidFeedAfterGrindDelay();
}

// RER/DER 기반 1회 급식량 계산
void PetFeeder::calculatePortion() {
  calculatePortionGrams(feedCount, feedTimes, petWeight, activityLevel, kcalPerKg);
}

// 서보모터 + 로드셀 기반 사료 투입
void PetFeeder::feedFood() {
  // initFeeder();  ← 제거: 생성자에서 이미 한 번 초기화했으므로 중복 호출 불필요
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
  soakingDone = true;  // ← 변수명 오타 수정 (isSoaking → soakingDone)
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


void PetFeeder::checkLiquidFeedAfterGrindDelay() {
    // 유동식 분쇄 후 일정 시간(30분) 지나면 서버로 잔여량 분석 요청
    if (!grindingDone || foodLevelChecked) return;

    unsigned long now = millis();
    if (now - grindingDoneTime >= 1800000) {  // 30분 경과
        Serial.println("⏱️ 분쇄 후 30분 경과 → 유동식 잔여량 서버 분석 요청");

        // 서버로 이미지 전송 후 JSON에서 remain_ratio 받기
        float ratio = detectLiquidFeedRatio();
        if (ratio < 0.0f) {
            Serial.println("❌ 유동식 분석 실패");
        } else {
            int percent = int(ratio * 100);
            Serial.printf("유동식 잔여율: %d%%\n", percent);

            // 20% 미만이면 건강 체크 알림
            if (percent < 20) {
                Serial.printf("⚠️ 반려동물이 유동식의 %d%%만 남겼습니다. 건강 체크가 필요합니다.\n", percent);
            }
        }

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
