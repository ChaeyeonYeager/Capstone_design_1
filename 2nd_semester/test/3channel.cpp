#include <Arduino.h>
#include <Servo.h>
#include <HX711.h>
#include <math.h>

// -------------------------------
// 채널 수
// -------------------------------
#define N_CHANNELS 3

// -------------------------------
// 핀 설정 (예시)
// - 서보: 10, 11, 12
// - HX711: DT=3,4,5 / SCK=2(공통)
// -------------------------------
const uint8_t SERVO_PINS[N_CHANNELS] = {10, 11, 12};
const uint8_t HX_DT_PINS[N_CHANNELS] = {3, 4, 5};
const uint8_t HX_SCK_SHARED = 2;

// -------------------------------
// 객체
// -------------------------------
Servo servos[N_CHANNELS];
HX711 hx[N_CHANNELS];

// -------------------------------
// 로드셀 보정값 (채널별로 개별 캘리브레이션 권장)
// -------------------------------
uint32_t CAL_OFFSET[N_CHANNELS] = {4294672803UL, 4294672803UL, 4294672803UL};
float CAL_SCALE[N_CHANNELS] = {-797.160888f, -797.160888f, -797.160888f};

// -------------------------------
// 그릇 무게/허용오차 (채널별)
// -------------------------------
float BOWL_WEIGHT[N_CHANNELS] = {100.0f, 100.0f, 100.0f};  // g
const float TOLERANCE[N_CHANNELS] = {2.0f, 2.0f, 2.0f};   // g

// -------------------------------
// 급식 계산용 변수 (모니터링 목적)
// -------------------------------
float RER = 0, DER = 0, portionGrams = 0;

// -------------------------------
// 상태
// -------------------------------
bool feedingActive[N_CHANNELS] = {false, false, false};
unsigned long t_motor_first[N_CHANNELS] = {0, 0, 0};

// -------------------------------
// 이름→슬롯 매핑 (없으면 slot=0)
// -------------------------------
const char* DOG_NAMES[N_CHANNELS] = {"small", "medium", "large"}; // 필요 시 변경

// -------------------------------
// 프로토타입
// -------------------------------
float calculatePortionGrams(int feedingCount, float dogWeight, float activeLvl, float calPerKg);
float getSuperStableWeight(int ch);
void runServoOnce(int ch);
void performFeeding(int ch, float targetGrams);
void handleSerial();

// -------------------------------
// 유틸: 이름→슬롯
// -------------------------------
int nameToSlot(const String& name) {
  for (int i = 0; i < N_CHANNELS; i++) {
    if (name.equalsIgnoreCase(DOG_NAMES[i])) return i;
  }
  return 0; // 기본
}

// -------------------------------
// setup
// -------------------------------
void setup() {
  Serial.begin(115200);
  while (!Serial) {}

  // 서보/HX711 초기화
  for (int i = 0; i < N_CHANNELS; i++) {
    servos[i].attach(SERVO_PINS[i]);
    delay(200);
    hx[i].begin(HX_DT_PINS[i], HX_SCK_SHARED);
    hx[i].set_offset(CAL_OFFSET[i]);
    hx[i].set_scale(CAL_SCALE[i]);
    hx[i].tare(); // 시작 시 영점
  }

  Serial.println();
  Serial.println("[START] 3채널 자동 급식 시스템 준비 완료");
  Serial.println("CSV 예시(권장): slot,name,score,size,weight,active,calPerKg,feedingCount");
  Serial.println("CSV 호환: name,score,size,weight,active,calPerKg,feedingCount (슬롯 자동매핑/없으면 0)");
}

// -------------------------------
// loop
// -------------------------------
void loop() {
  handleSerial();
}

// -------------------------------
// 급식량 계산
// -------------------------------
float calculatePortionGrams(int feedingCount, float dogWeight, float activeLvl, float calPerKg) {
  RER = 70.0f * pow(dogWeight, 0.75f);
  DER = RER * activeLvl;
  float dailyGrams = (DER / calPerKg) * 1000.0f;
  portionGrams = dailyGrams / feedingCount;

  Serial.println("===== 급식량 계산 =====");
  Serial.print("체중(kg): "); Serial.println(dogWeight, 3);
  Serial.print("활동계수: "); Serial.println(activeLvl, 3);
  Serial.print("급여 횟수: "); Serial.println(feedingCount);
  Serial.print("사료 kcal/kg: "); Serial.println(calPerKg, 3);
  Serial.print("RER: "); Serial.print(RER, 3); Serial.println(" kcal");
  Serial.print("DER: "); Serial.print(DER, 3); Serial.println(" kcal");
  Serial.print("총 하루 사료량: "); Serial.print(dailyGrams, 3); Serial.println(" g");
  Serial.print("1회 사료량: "); Serial.print(portionGrams, 3); Serial.println(" g");
  Serial.println("======================");

  return portionGrams;
}

// -------------------------------
// HX711 안정 무게 측정 (채널별) + 그릇무게 보정
// -------------------------------
float getSuperStableWeight(int ch) {
  const int numReadings = 10;
  float readings[numReadings];
  float sum = 0;

  for (int i = 0; i < numReadings; i++) {
    readings[i] = hx[ch].get_units();
    delay(30);
  }

  for (int i = 0; i < numReadings; i++) sum += readings[i];
  float avg = sum / numReadings;

  float filteredSum = 0;
  int filteredCount = 0;
  for (int i = 0; i < numReadings; i++) {
    if (fabs(readings[i] - avg) < 0.02f) {
      filteredSum += readings[i];
      filteredCount++;
    }
  }

  float stable = (filteredCount > 0) ? (filteredSum / filteredCount) : avg;
  stable -= BOWL_WEIGHT[ch]; // 그릇 무게 보정
  if (stable < 0) stable = 0;

  Serial.print("[WEIGHT ch="); Serial.print(ch);
  Serial.print("] 안정 무게(보정 후): "); Serial.print(stable, 3); Serial.println(" g");

  return stable;
}

// -------------------------------
// 서보 1회 동작 (채널별)
// -------------------------------
void runServoOnce(int ch) {
  Serial.print("[MOTOR ch="); Serial.print(ch); Serial.println("] 서보 동작 시작");
  for (int a = 0; a <= 180; a++) {
    servos[ch].write(a);
    delay(5);
  }
  delay(100);
  for (int a = 180; a >= 0; a--) {
    servos[ch].write(a);
    delay(5);
  }
  Serial.print("[MOTOR ch="); Serial.print(ch); Serial.println("] 서보 동작 종료");
}

// -------------------------------
// 목표 사료량까지 반복 급여 (채널별)
// -------------------------------
void performFeeding(int ch, float targetGrams) {
  Serial.print("===== 자동 급여 시작 (ch="); Serial.print(ch); Serial.println(") =====");
  feedingActive[ch] = true;
  t_motor_first[ch] = 0;

  float cur = getSuperStableWeight(ch);
  float diff = targetGrams - cur;

  if (diff <= TOLERANCE[ch]) {
    Serial.println("이미 목표량 이상이므로 급식 생략");
    feedingActive[ch] = false;
    Serial.println("===========================");
    Serial.println("[DONE]");
    return;
  }

  int cycle = 0;
  while (diff > TOLERANCE[ch]) {
    cycle++;
    if (t_motor_first[ch] == 0) {
      t_motor_first[ch] = millis();
      Serial.print("[METRIC ch="); Serial.print(ch);
      Serial.print("] Motor first start time = ");
      Serial.print(t_motor_first[ch]); Serial.println(" ms");
    }

    Serial.print("[CYCLE ch="); Serial.print(ch);
    Serial.print(" #"); Serial.print(cycle); Serial.println("]");
    runServoOnce(ch);
    delay(1000);

    cur = getSuperStableWeight(ch);
    diff = targetGrams - cur;

    Serial.print("남은 목표량: "); Serial.print(diff, 3); Serial.println(" g");
  }

  feedingActive[ch] = false;
  Serial.println("목표 사료량 도달 → 급식 완료");
  Serial.println("===========================");
  Serial.println("[DONE]");
}

// -------------------------------
// 시리얼 수신 처리
// -------------------------------
void handleSerial() {
  if (!Serial.available()) return;
  String data = Serial.readStringUntil('\n');
  data.trim();
  if (data.length() == 0) return;

  Serial.print("[RECV] "); Serial.println(data);

  // 쉼표 위치 수집
  int commaPos[8] = {-1, -1, -1, -1, -1, -1, -1, -1};
  int count = 0;
  for (int i = 0; i < (int)data.length() && count < 8; i++) {
    if (data[i] == ',') commaPos[count++] = i;
  }

  // 필드 파싱 함수
  auto field = [&](int startIdx, int endIdxExclusive)->String {
    if (startIdx >= (int)data.length()) return "";
    int end = min(endIdxExclusive, (int)data.length());
    if (end <= startIdx) return "";
    String s = data.substring(startIdx, end);
    s.trim();
    return s;
  };

  // 필드 파싱
  int slot = 0;
  String name, size;
  float score = 0, weight = 0, activeLvl = 1.0, calPerKg = 350.0;
  int feedingCount = 2;

  if (count >= 7) { // slot 포함 포맷
    String slotStr = field(0, commaPos[0]);
    name = field(commaPos[0] + 1, commaPos[1]);
    String scoreStr = field(commaPos[1] + 1, commaPos[2]);
    size = field(commaPos[2] + 1, commaPos[3]);
    String wtStr = field(commaPos[3] + 1, commaPos[4]);
    String actStr = field(commaPos[4] + 1, commaPos[5]);
    String calStr = field(commaPos[5] + 1, commaPos[6]);
    String cntStr = field(commaPos[6] + 1, data.length());

    slot = slotStr.toInt();
    score = scoreStr.toFloat();
    weight = wtStr.toFloat();
    activeLvl = actStr.toFloat();
    calPerKg = calStr.toFloat();
    feedingCount = cntStr.toInt();
    if (slot < 0 || slot >= N_CHANNELS) slot = nameToSlot(name);
  }
  else if (count >= 6) { // slot 없는 호환 포맷
    name = field(0, commaPos[0]);
    String scoreStr = field(commaPos[0] + 1, commaPos[1]);
    size = field(commaPos[1] + 1, commaPos[2]);
    String wtStr = field(commaPos[2] + 1, commaPos[3]);
    String actStr = field(commaPos[3] + 1, commaPos[4]);
    String calStr = field(commaPos[4] + 1, commaPos[5]);
    String cntStr = field(commaPos[5] + 1, data.length());

    score = scoreStr.toFloat();
    weight = wtStr.toFloat();
    activeLvl = actStr.toFloat();
    calPerKg = calStr.toFloat();
    feedingCount = cntStr.toInt();
    slot = nameToSlot(name);
  }
  else {
    Serial.println("[ERROR] CSV 필드 수가 부족합니다.");
    return;
  }

  // 요약 출력
  Serial.println("========== 급식 명령 수신 ==========");
  Serial.print("채널(slot): "); Serial.println(slot);
  Serial.print("강아지: "); Serial.println(name);
  Serial.print("체급: "); Serial.println(size);
  Serial.print("체중: "); Serial.print(weight); Serial.println(" kg");
  Serial.print("활동수준: "); Serial.println(activeLvl);
  Serial.print("사료 kcal/kg: "); Serial.println(calPerKg);
  Serial.print("급여횟수: "); Serial.println(feedingCount);
  Serial.println("=================================");

  float target = calculatePortionGrams(feedingCount, weight, activeLvl, calPerKg);
  performFeeding(slot, target);
}
