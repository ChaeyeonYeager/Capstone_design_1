#include <feeder.h>
#include <feeding_calc.h>

// ✅ feeder 파일 set up 함수
// 로드셀, 서보모터 초기화화
void initFeeder(){
  Serial.begin(115200);
  hx711.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  hx711.set_scale(calibration_factor);
  hx711.tare();  // 초기 영점(0) 설정

  myServo.attach(SERVOPIN);
  myServo.write(0);  // 서보 닫기

  randomSeed(analogRead(0));  // 매번 다른 랜덤값 생성
}

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


// ✅ 사료를 실제로 투입하는 함수 (서보 + 로드셀)
void executeFeeding(int index) {
  Serial.println("[" + getTimeString(rtc.now()) + "] 급식 시작"); // 현재 시각 로그 출력(디버깅용용)

 portionGrams=calculatePortionGrams(); // !!!랜덤값 집어넣기!!!

  servo.write(90);         // 투입구 열기
  delay(500);             // 사료 투하 대기 (0.5초)

  float target = portionGrams;        // 목표 사료량
  float minAccept = target * 0.95;    // 허용 하한 (95%)
  float maxAccept = target * 1.05;    // 허용 상한 (105%)
  float weight = 0;

  // ✅ 로드셀 무게가 목표 범위 안에 들어올 때까지 대기
  int timeout = 0;
  while (true) {
    weight = getSuperStableWeight(); // 로드셀로 현재 그릇+사료 무게 측정정
    if (weight >= minAccept && weight <= maxAccept) {
      break;
    }

    // 목표 무게에 너무 오래 도달하지 못하면 안전 탈출
    timeout++;
    if (timeout > 50) { // 예: 약 1.5초 후 강제 종료
      Serial.println("⚠️ 무게 측정 타임아웃. 급식 종료");
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
  for (int i = 0; i < MAX; i++) {
    feedDoneToday[i] = false;
  }
  isFoodInputDone = false;
}

// ✅ 급식 완료 여부 반환
bool isFeedingDone() {
  return isFoodInputDone;
}
