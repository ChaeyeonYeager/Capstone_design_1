#include <../feeding_calc.h>

// ✅ 함수 선언
int foodWeightPerMeal_calc(int feedingCount,string feedTimes[], float dogWeight, float activeLvl, float calPerKg); // 사료량 계산 함수

void setup() {
  Serial.begin(9600);

  foodWeightPerMeal = foodWeightPerMeal_calc(feedCount,feedTimes, weight, activityLevel, kcalPerKg);
}

float foodWeightPerMeal_calc(int feedingCount, string feedTimes[], float dogWeight, float activeLvl, float calPerKg) {
  // ✅ RER 계산: 70 * (체중^0.75)
  RER = 70 * pow(dogWeight, 0.75);

  // ✅ DER 계산: RER * 활동지수
  DER = RER * activeLvl;

  // ✅ DER을 1회 급여량으로 변환: ((DER / calPerKg) * 1000) / 급여횟수
  float foodWeightPerMeal = ((DER / calPerKg) * 1000.0) / feedingCount;

      // 📤 결과 출력
      Serial.println("========================");
      Serial.println("🔬 급식량 계산 테스트");
      Serial.print("체중: "); Serial.print(dogWeight); Serial.println(" kg");
      Serial.print("활동지수: "); Serial.println(activeLvl);
      Serial.print("급여 횟수: "); Serial.println(feedingCount);
      Serial.print("RER (기초 에너지): "); Serial.print(RER); Serial.println(" kcal");
      Serial.print("DER (하루 필요 에너지): "); Serial.print(DER); Serial.println(" kcal");
      Serial.print("1회 사료량: "); Serial.print(foodWeightPerMeal); Serial.println(" g");
      Serial.println("========================");

  // ✅ 1회 급여량 계산: ((DER / calPerKg) * 1000) / 급여횟수
  portionGrams=foodWeightPerMeal; 
  return foodWeightPerMeal;

}

void loop() {
  
}