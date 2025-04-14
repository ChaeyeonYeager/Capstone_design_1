#include "FlowCheck.h"

FlowSensor* instance = nullptr; // 센서가 펄스를 보낼 때마다 알려줄 것임..

// :: -> FlowCheck.h에 있는 함수

// 처음 초기화 할 때 
FlowSensor::FlowSensor(uint8_t interruptPin, float pulsesPerLiter) : // 핀 번호, 펄스 당 리터 수 저장 
      pin(interruptPin), pulseCount(0), pulsesPerLiter(pulsesPerLiter),
      lastUpdateTime(0), flowRate(0), totalMilliLiters(0) {
    instance = this; // 인터럽트에서 이 인스턴스 참조할 수 있도록 저장
                     // 클래스 멤버 함수에 접근하기 위해 전역 변수를 이용하여 현재 객체에 접근근
}

// 초기화 함수: 핀을 입력으로 설정하고 인터럽트 등록
void FlowSensor::begin() {
    pinMode(pin, INPUT_PULLUP); // 핀을 풀업 저항과 함께 입력 모드로 설정

    // attachInterrupt(인터럽트 번호, 실행할 함수(전역 함수만 받을 수 있음), 트리거 조건)

    // digitalPinToInterrupt(pin) ==> 핀 번호를 인터럽트 번호로 ==> attachInterrupt()함수가 인터럽트 번호를 요구하기 때문.
    // []() {instance->pulseISR();}, RISING ==> 클래스 내부 함수 바로 못 쓰므로 instance 포인터를 통해 pulseISR() 간접 호출, RISING 신호에 pulseISR() 함수 실행
    attachInterrupt(digitalPinToInterrupt(pin), []() {instance->pulseISR();}, RISING);
    lastUpdateTime = millis(); // 흐른 시각 기록
}

// 펄스 수 하나 증가 (인터럽스 발생 시 호출)
void FlowSensor::pulseISR() {
    pulseCount++;
}

// 1초마다 호출해서 유량 계산
void FlowSensor::update() {
    unsigned long currentTime = millis(); // 현재 시간(ms)을 기록록
    unsigned long deltaTime = currentTime - lastUpdateTime; // 현재 시간과 마지막 시간을 빼서 얼마나 시간이 지났는지?를 기록록

    // 1초마다 계산하기 위해... 1초 이상 흘렀으면 실행.
    if (deltaTime >= 1000) { 
        noInterrupts(); // 인터럽트 잠깐 중단 (안전하게 값 읽기 위해서)
        uint32_t count = pulseCount; // 펄스 수 임시 저장
        pulseCount = 0; // 펄스 수 초기화
        interrupts(); // 인터럽트 다시 켜기

        // 유량 계산: (펄스 수 / 리터 당 펄스 수) x 1000 = mL/s
        // count / pulsesPerLiter => 몇 리터인지 계산
        // * 1000 => 리터를 밀리미터로 계산산
        flowRate = (count / pulsesPerLiter) * 1000.0; // L/s → mL/s
        totalMilliLiters += flowRate; // 누적 유량 추가
        lastUpdateTime = currentTime; // 마지막 업데이트 시간 갱신
    }
}

// 현재 초당 유량 반환
float FlowSensor::getFlowRate() {
    return flowRate;
}

// 현재까지 총 유량 반환
float FlowSensor::getTotalVolume() {
    return totalMilliLiters;
}

// 유량 초기화
void FlowSensor::resetVolume() {
    totalMilliLiters = 0;
}
