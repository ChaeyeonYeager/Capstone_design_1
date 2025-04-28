#include <HX711.h>  // 로드셀(무게측정 센서) 라이브러리 추가

HX711 hx711;  // 로드셀 객체 선언
float calibration_factor = -28000;  // 로드셀 교정(보정) 값 변수
#define LOADCELL_DOUT_PIN 2  // 로드셀 데이터 연결 핀 번호
#define LOADCELL_SCK_PIN 3   // 로드셀 Clock 연결 핀 번호

void setup() {
  Serial.begin(115200);  // 시리얼 통신 속도 설정
  hx711.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);  // 로드셀 활성화
  hx711.set_scale(calibration_factor);  // 초기 보정값 적용
  hx711.tare();  // 영점(0kg) 설정

  Serial.println("Ready. Send new calibration factor if needed.");
}

// 이동 평균 기반 정밀 측정 함수
float getSuperStableWeight() {
  const int numReadings = 20;  // 20번 측정
  float readings[numReadings];
  float sum = 0;

  // 측정값 모두 저장
  for (int i = 0; i < numReadings; i++) {
    readings[i] = hx711.get_units();
    delay(30);  // 각 측정 사이 짧은 대기 (속도/정확도 균형)
  }

  // 1차 평균 계산
  for (int i = 0; i < numReadings; i++) {
    sum += readings[i];
  }
  float average = sum / numReadings;

  // 2차 필터링: 평균 기준 ±20g(0.02kg) 이내만 다시 평균
  float filteredSum = 0;
  int filteredCount = 0;
  for (int i = 0; i < numReadings; i++) {
    if (abs(readings[i] - average) < 0.02) {  // 20g 이내
      filteredSum += readings[i];
      filteredCount++;
    }
  }

  if (filteredCount > 0) {
    return filteredSum / filteredCount;  // 필터링 후 재평균
  } else {
    return average;  // 통과된 데이터가 없으면 1차 평균 사용
  }
}

void loop() {
  // 시리얼 입력으로 보정값 수정 가능
  if (Serial.available() > 0) {
    String str = Serial.readStringUntil('\n');
    calibration_factor = str.toFloat();
    hx711.set_scale(calibration_factor);
    Serial.print("New Calibration Factor: ");
    Serial.println(calibration_factor);
  }

  // 슈퍼 안정화된 무게 읽기
  float weight = getSuperStableWeight();  // 단위: kg

  // 결과 출력 (g 단위 변환)
  Serial.print("Weight : ");
  Serial.print(weight * 1000, 2);  // g단위 출력, 소수점 2자리
  Serial.print(" g\t Calibration : ");
  Serial.println(calibration_factor);

  delay(500);  // 0.5초마다 반복
}
