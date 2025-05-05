#include <feeder.h>

void runFeedingSchedule() {
  DateTime now = rtc.now();
  String currentTime = getTimeString(now);

  for (int i = 0; i < feedCount; i++) {
    if (!feedDoneToday[i] && currentTime == feedTimes[i]) {
      executeFeeding(i);
    }
  }
}

String getTimeString(DateTime now) {
  char buf[6];
  sprintf(buf, "%02d:%02d", now.hour(), now.minute());
  return String(buf);
}

void executeFeeding(int index) {
  Serial.println("[" + getTimeString(rtc.now()) + "] 급식 시작");

  servo.write(90);       // 투입구 개방
  delay(1000);           // 사료 투하 시간 확보

  float target = portionGrams;
  float minAccept = target * 0.95;
  float maxAccept = target * 1.05;
  float weight = 0;

  while (true) {
    weight = scale.get_units();
    if (weight >= minAccept && weight <= maxAccept) {
      break;
    }
  }

  servo.write(0);                   // 투입구 닫기
  feedDoneToday[index] = true;
  isFoodInputDone = true;

  Serial.println("급식 완료: " + String(weight, 1) + "g / 목표 " + String(target, 1) + "g");
}

void resetDailyFeeding() {
  for (int i = 0; i < 6; i++) {
    feedDoneToday[i] = false;
  }
  isFoodInputDone = false;
}

bool isFeedingDone() {
  return isFoodInputDone;
}
