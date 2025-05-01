#include <Arduino.h>

// :: -> FlowCheck.h에 있는 함수

uint8_t pin; // 연결 된 핀 번호 
volatile uint32_t pulseCount; // 받은 펄스 수 (인터럽트로 증가)
float pulsesPerLiter; // 리터당 펄스 수 (약 450)
unsigned long lastUpdateTime; // 마지막으로 유량을 계산한 시간
float flowRate; // 초당 흐르는 유랑 (mL/s)
float totalMilliLiters; // 전체 누적 유랑 (mL)


// 처음 초기화 할 때 
void FlowSensor(uint8_t interruptPin, float PerLiter) { // 핀 번호, 펄스 당 리터 수 저장 
      pin = interruptPin;
      pulseCount = 0;
      pulsesPerLiter = PerLiter;
      lastUpdateTime = 0;
      flowRate = 0;
      totalMilliLiters = 0;
}

// 초기화 함수: 핀을 입력으로 설정하고 인터럽트 등록
void begin() {
    pinMode(pin, INPUT_PULLUP); // 핀을 풀업 저항과 함께 입력 모드로 설정

    // attachInterrupt(인터럽트 번호, 실행할 함수(전역 함수만 받을 수 있음), 트리거 조건)

    // digitalPinToInterrupt(pin) ==> 핀 번호를 인터럽트 번호로 ==> attachInterrupt()함수가 인터럽트 번호를 요구하기 때문.
    // []() {instance->pulseISR();}, RISING ==> 클래스 내부 함수 바로 못 쓰므로 instance 포인터를 통해 pulseISR() 간접 호출, RISING 신호에 pulseISR() 함수 실행
    //attachInterrupt(digitalPinToInterrupt(pin), []() {pulseISR();}, RISING);
    lastUpdateTime = millis(); // 흐른 시각 기록
}

// 펄스 수 하나 증가 (인터럽스 발생 시 호출)
void pulseISR() {
    pulseCount++;
}

// 1초마다 호출해서 유량 계산
void flowUpdate() {
    unsigned long currentTime = millis(); // 현재 시간(ms)을 기록
    unsigned long deltaTime = currentTime - lastUpdateTime; // 현재 시간과 마지막 시간을 빼서 얼마나 시간이 지났는지?를 기록록

    // 1초마다 계산하기 위해... 1초 이상 흘렀으면 실행.
    if (deltaTime >= 1000) { 
        noInterrupts(); // 인터럽트 잠깐 중단 (안전하게 값 읽기 위해서)
        uint32_t count = pulseCount; // 펄스 수 임시 저장
        pulseCount = 0; // 펄스 수 초기화
        interrupts(); // 인터럽트 다시 켜기

        // 유량 계산: (펄스 수 / 리터 당 펄스 수) x 1000 = mL/s
        // count / pulsesPerLiter => 몇 리터인지 계산
        // * 1000 => 리터를 밀리미터로 계산
        flowRate = (count / pulsesPerLiter) * 1000.0; // L/s → mL/s
        totalMilliLiters += flowRate; // 누적 유량 추가
        lastUpdateTime = currentTime; // 마지막 업데이트 시간 갱신
    }
}

// 현재 초당 유량 반환
float getFlowRate() {
    return flowRate;
}

// 현재까지 총 유량 반환
float getTotalVolume() {
    return totalMilliLiters;
}

// 유량 초기화
void resetVolume() {
    totalMilliLiters = 0;
}

// 타겟 유량 달성 완료 함수
bool targetWater(){
    // (물 정량값 - 불리기 위해 넣은 값) 계산한 변수 받아서
    // 그 변수(targetVolume)와 totalMilliLiters 비교
    // if (targetVolume <= totalMilliLiters) {return true;} else {return false;} 
    
    // 일단 return true
    return true;
}
