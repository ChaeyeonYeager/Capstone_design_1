#include <feeder.h>

// 급식시간이 되었고 그 시간에 급식을 하지 않았다면 배식
void checkAndFeed() {
  DateTime now = rtc.now();
  String currentTime = getTimeString(now); // 현재 시간 문자열

  for (int i = 0; i < feedCount; i++) {
    if (!feedDoneToday[i] && currentTime == feedTimes[i]) {
      feedPortion(i);
    }
  }
}

// 현재 시간을 "HH:MM" 형식으로 반환 -> 현재시간 확인 함수
String getTimeString(DateTime now) {
  char buf[6];
  sprintf(buf, "%02d:%02d", now.hour(), now.minute());
  return String(buf);
}

// 실제 사료를 배급하는 함수
void feedPortion(int index) {
  Serial.println("[" + getTimeString(rtc.now()) + "] 급식 시작");

  servo.write(90);         // 투입구 개방
  delay(1000);             // 사료 투하 시간 확보 ***상의 후 시간 정하기***

  float target = foodWeightPerMeal;
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
  isFoodInputDone=true;

  Serial.println("급식 완료: " + String(weight, 1) + "g / 목표 " + String(target, 1) + "g");
}

// 하루가 시작되면 급식 완료 배열을 초기화
void resetFeedingFlags() {
  for (int i = 0; i < 6; i++) {
    feedDoneToday[i] = false;
  }
  isFoodInputDone=false;
}

// 배급 상태 확인
bool isFoodInputDoneState(){
  return isFoodInputDone;
}

void loop() {
  
}
