#ifndef PET_FEEDER_H
#define PET_FEEDER_H

#include <Arduino.h>
#include "feeding_calc.h"
#include "feeder.h"
#include "WaterControl.h"
#include "feed_grinder.h"
#include "feed_level_check.h"
#include "LiquidFeedDetection.h"


#define MAX 6  // 최대 급여 횟수

/*
===============================
🐶 PetFeeder 클래스 전체 흐름
===============================
1. 생성자에서 필요한 값(이름, 몸무게, 활동지수 등)과 급식 시간을 받아 초기화
2. 자동으로 급식량 계산 (RER/DER 기반)
3. runFeedingRoutine() 호출 시 아래 순서대로 동작
   - 1) 사료 투입 (서보 + 로드셀)
   - 2) 1:1 비율 물 주입 → 불림
   - 3) 점도 설정에 따라 추가 물 주입 (1:1.5 또는 1:2)
   - 4) 사료 분쇄 (모터)
   - 5) 완료 알림
===============================
*/

class PetFeeder {
public:
  // 생성자: 반려동물 데이터 및 급식 설정 초기화
  PetFeeder(String name, float weight, int age, int feedCount, String feedTimes[],
            float activityLevel, int kcalPerKg, int viscosityLevel);

  // 급식 루틴 실행 (시간 도달 시 호출됨)
  void runFeedingRoutine();
  void checkFoodLevelAfterGrindDelay();  // ✅ 30분 후 사료 잔량 체크
  // 상태 반환
  bool isFeedingComplete() const;
  bool isSoakingComplete() const;
  bool isGrindingComplete() const;

  // 급식 정보 조회
  int getFeedCount() const;
  String getFeedTime(int index) const;

private:
  // 기본 정보
  String petName;
  float petWeight;
  int petAge;

  // 급식 설정값
  int feedCount;                   // 하루 급여 횟수
  String feedTimes[MAX];           // 급여 시간 목록 (HH:MM)
  float activityLevel;             // 활동 지수
  int kcalPerKg;                   // kg당 필요 칼로리
  int viscosityLevel;              // 점도 단계 (0: 1:1, 1: 1:1.5, 2: 1:2)

  // 물 계산용
  float baseWaterVolume;          // 1차 불림용 물 (사료량과 1:1)
  float extraWaterVolume;         // 점도 조절용 물 (사료량 * 추가 비율)

  // 상태 플래그
  bool feedingDone;
  bool soakingDone;
  bool grindingDone;


  unsigned long grindingDoneTime;   // ✅ 분쇄 완료 시각
  bool foodLevelChecked;            // ✅ 잔량 체크 완료 여부

  // 내부 동작 함수
  void calculatePortion();        // 사료량 계산
  void feedFood();                // 사료 급여
  void soakFood();                // 1차 물 주입 + 불림
  void addExtraWater();           // 점도 조절용 물 주입
  void grindFood();               // 분쇄 (모터 작동)
  void checkLiquidFeedAfterGrindDelay();
};

#endif // PET_FEEDER_H
