// src/PetFeeder.h
#ifndef PET_FEEDER_H
#define PET_FEEDER_H

#include <Arduino.h>
#include "a_feeding_calc.h"
#include "b_feeder.h"
#include "Watercontrol.h"
#include "c_foodSoaking.h"
#include "d_addExtraWaterProcess.h"
#include "e_feed_grinder.h"
#include "f_feed_level_check.h"
#include "globals.h"

#define MAX 6  // 최대 급여 횟수

class PetFeeder {
public:
  PetFeeder(String name,
            float weight,
            int age,
            int feedCount,
            String feedTimes[],
            float activityLevel,
            int kcalPerKg,
            int viscosityLevel);

  void runFeedingRoutine();
  void checkFoodLevelAfterGrindDelay();

private:
  // 펫 정보
  String petName;
  float petWeight;
  int petAge;

  // 급식 설정
  int feedCount;
  String feedTimes[MAX];
  float activityLevel;
  int kcalPerKg;
  int viscosityLevel;

  // 계산된 양
  float portionGrams;
  float baseWaterVolume;
  float extraWaterVolume;

  // 상태 플래그
  bool feedingDone;
  bool soakingDone;
  bool grindingDone;
  unsigned long grindingDoneTime;
  bool foodLevelChecked;

  // 내부 동작
  void calculatePortion();
  void feedFood();
  void soakFood();
  void addExtraWater();
  void grindFood();
};

#endif // PET_FEEDER_H
