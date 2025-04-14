// Feeder.cpp - 자동 급식기 제어 클래스 구현 파일
#include "./feeder.h"

// 생성자: 핀 번호 초기화 및 객체 생성
Feeder::Feeder(int doutPin, int clkPin, int servoPin, int rxPin, int txPin)
  : BT(rxPin, txPin), activityFactor(1.6) {
  scale.begin(doutPin, clkPin);
  servo.attach(servoPin);
}

void Feeder::setup() {
  Serial.begin(9600);
  BT.begin(9600);
  rtc.begin();

  // RTC가 초기화되지 않았다면 현재 시간으로 설정
  if (rtc.lostPower()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  // 로드셀 설정
  scale.set_scale(2280.f);  // 보정 계수 (환경에 따라 측정 필요)
  scale.tare();             // 초기 0점 설정

  servo.write(0);           // 서보모터 닫힘 상태로 초기화

  resetFeedingFlags();      // 급식 상태 초기화
}

void Feeder::loop() {
  receiveBluetoothData();   // 설정 데이터 수신

  DateTime now = rtc.now();
  String currentTime = getTimeString(now); // 현재 시간 문자열

  // 급식 시간과 비교하여 자동 급식 수행
  for (int i = 0; i < feedCount; i++) {
    if (!feedDoneToday[i] && currentTime == feedTimes[i]) {
      feedPortion(i);
    }
  }

  delay(1000); // 1초 대기 (효율 조절)
}

void Feeder::receiveBluetoothData() {
  if (BT.available()) {
    String data = BT.readStringUntil('\n');
    parseBluetoothData(data); // 수신된 설정 파싱
  }
}

void Feeder::parseBluetoothData(String input) {
  input.trim();
  int idx = 0;
  String parts[15];

  while (input.length() > 0) {
    int comma = input.indexOf(',');
    if (comma == -1) {
      parts[idx++] = input;
      break;
    }
    parts[idx++] = input.substring(0, comma);
    input = input.substring(comma + 1);
  }

  petName = parts[0];
  age = parts[1].toInt();
  weight = parts[2].toFloat();
  feedCount = parts[3].toInt();

  for (int i = 0; i < feedCount; i++) {
    feedTimes[i] = parts[4 + i];
  }

  activityLevel = parts[4 + feedCount];
  kcalPerKg = parts[5 + feedCount].toInt();

  // 활동량 수준에 따라 계수 설정
  if (activityLevel == "낮음") activityFactor = 1.2;
  else if (activityLevel == "중간") activityFactor = 1.6;
  else if (activityLevel == "높음") activityFactor = 2.0;

  calculatePortion();       // 1회 사료량 계산
  resetFeedingFlags();      // 급식 상태 초기화

  Serial.println("설정 완료: " + petName);
}

// 사료량 계산: 하루 총 에너지 → 사료량 → 회당 사료량 계산
void Feeder::calculatePortion() {
  float kcalPerGram = kcalPerKg / 1000.0;
  float base = 30 * weight + 70;
  float dailyCalories = base * activityFactor;
  float dailyGrams = dailyCalories / kcalPerGram;
  portionGrams = dailyGrams / feedCount;
}

// 현재 시간을 "HH:MM" 형식으로 반환
String Feeder::getTimeString(DateTime now) {
  char buf[6];
  sprintf(buf, "%02d:%02d", now.hour(), now.minute());
  return String(buf);
}

// 실제 사료를 배급하는 함수
void Feeder::feedPortion(int index) {
  Serial.println("[" + getTimeString(rtc.now()) + "] 급식 시작");

  servo.write(90);         // 투입구 개방
  delay(3000);             // 사료 투하 시간 확보

  float target = portionGrams;
  float minAccept = target * 0.95;
  float maxAccept = target * 1.05;

  float weight = 0;

  // 로드셀로 목표 무게 달성까지 모니터링
  while (true) {
    weight = scale.get_units();
    if (weight >= minAccept && weight <= maxAccept) {
      break;
    }
  }

  servo.write(0);                  // 투입구 닫기
  feedDoneToday[index] = true;    // 급식 완료 표시

  Serial.println("급식 완료: " + String(weight, 1) + "g / 목표 " + String(target, 1) + "g");
}

// 하루가 시작되면 급식 완료 배열을 초기화
void Feeder::resetFeedingFlags() {
  for (int i = 0; i < 6; i++) {
    feedDoneToday[i] = false;
  }
}
