#include <Arduino.h>

// ✅ 변수 선언
int feedingCount;           // 하루 급여 횟수
float dogWeight;            // 강아지 체중 (kg)
float activeLvl;            // 활동지수
float calPerKg;             // 1kg당 사료 칼로리 (kcal)

// ✅ 결과 변수
float RER;                  // 기초 에너지 요구량 (kcal)
float DER;                  // 하루 에너지 요구량 (kcal)
float foodWeightPerMeal;    // 1회 급여 사료량 (g)

// ✅ 함수 선언
int foodWeightPerMeal_calc(int feedingCount, float dogWeight, float activeLvl, float calPerKg); // 사료량 계산 함수

void setup() {
  Serial.begin(9600);

  // 🔢 랜덤 테스트값 입력
  feedingCount = 2;           // 하루 2회 급여
  dogWeight = random(30, 80) / 10.0; // 3.0kg ~ 7.9kg 사이 무작위 체중
  activeLvl = 1.6;            // 중성화된 성견
  calPerKg = 3600;            // 1kg당 사료 칼로리 (예: 3600 kcal/kg)

  foodWeightPerMeal = foodWeightPerMeal_calc(feedingCount, dogWeight, activeLvl, calPerKg);
}

void loop() {
  // 반복 없음
}

int foodWeightPerMeal_calc(int feedingCount, float dogWeight, float activeLvl, float calPerKg) {
  // ✅ RER 계산: 70 * (체중^0.75)
  float RER = 70 * pow(dogWeight, 0.75);

  // ✅ DER 계산: RER * 활동지수
  float DER = RER * activeLvl;

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
  return foodWeightPerMeal;

}