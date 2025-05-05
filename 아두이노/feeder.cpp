#include <feeder.h>

// ✅ 급식 시간에 도달하면 사료 급여 실행
void runFeedingSchedule() {
  DateTime now = rtc.now();                      // 현재 시각
  String currentTime = getTimeString(now);       // "HH:MM" 형식 문자열

  for (int i = 0; i < feedCount; i++) {
    if (!feedDoneToday[i] && currentTime == feedTimes[i]) {
      executeFeeding(i);                         // 아직 급여 안 했고, 시간 일치 시 실행
    }
  }
}

// ✅ DateTime 객체를 "HH:MM" 형식 문자열로 변환
String getTimeString(DateTime now) {
  char buf[6];
  sprintf(buf, "%02d:%02d", now.hour(), now.minute());
  return String(buf);
}

// ✅ 사료를 실제로 투입하는 함수 (서보 + 로드셀)
void executeFeeding(int index) {
  Serial.println("[" + getTimeString(rtc.now()) + "] 급식 시작");

  servo.write(90);         // 투입구 열기
  delay(1000);             // 사료 투하 대기 (1초)

  float target = portionGrams;        // 목표 사료량
  float minAccept = target * 0.95;    // 허용 하한 (95%)
  float maxAccept = target * 1.05;    // 허용 상한 (105%)
  float weight = 0;

  // ✅ 로드셀 무게가 목표 범위 안에 들어올 때까지 대기
  while (true) {
    weight = scale.get_units();
    if (weight >= minAccept && weight <= maxAccept) {
      break;
    }
  }

  servo.write(0);                   // 투입구 닫기
  feedDoneToday[index] = true;     // 급여 완료 플래그 설정
  isFoodInputDone = true;

  Serial.println("급식 완료: " + String(weight, 1) + "g / 목표 " + String(target, 1) + "g");
}

// ✅ 하루 시작 시 모든 급식 완료 플래그 초기화
void resetDailyFeeding() {
  for (int i = 0; i < 6; i++) {
    feedDoneToday[i] = false;
  }
  isFoodInputDone = false;
}

// ✅ 급식 완료 여부 반환
bool isFeedingDone() {
  return isFoodInputDone;
}
