#include <Arduino.h>
#include "FlowCheck.h"

// ===== 유량 센서 설정 =====
#define FLOW_SENSOR_PIN 2         // 유량 센서가 연결된 핀 번호
#define PULSES_PER_LITER 450.0    // 1리터당 발생하는 펄스 수 (센서 스펙에 따라 다름)

// ===== 내부 측정 변수 =====
volatile uint32_t pulseCount;     // 인터럽트로 측정된 펄스 수
unsigned long lastUpdateTime;     // 마지막 유량 갱신 시간
float flowRate;                   // 초당 유량 (mL/s)
float totalMilliLiters;           // 누적 유량 (mL)

// ===== 목표 유량 설정 변수 =====
float targetVolume = 100.0;       // 목표 물 주입량 (mL) — 기본값 100mL

// ✅ 외부에서 목표 유량을 설정하는 함수
void setTargetVolume(float mL) {
    targetVolume = mL;
}

// ✅ 유량 센서 초기화 함수
void initFlowSensor() {
    pulseCount = 0;
    lastUpdateTime = 0;
    flowRate = 0;
    totalMilliLiters = 0;
}

void beginFlowSensor() {
    pinMode(FLOW_SENSOR_PIN, INPUT_PULLUP);  // 풀업 입력으로 핀 설정
    attachInterrupt(digitalPinToInterrupt(FLOW_SENSOR_PIN), pulseISR, RISING);  // 상승엣지에서 인터럽트 발생
    lastUpdateTime = millis();  // 초기 시간 저장
}

// ✅ 펄스가 들어올 때마다 1씩 증가 (인터럽트 함수)
void pulseISR() {
    pulseCount++;
}

// ✅ 유량 계산 함수 (1초 간격으로 호출해야 정확)
void flowUpdate() {
    unsigned long currentTime = millis();
    unsigned long deltaTime = currentTime - lastUpdateTime;

    if (deltaTime >= 1000) { // 1초마다 계산
        noInterrupts();           // 인터럽트 중단 (데이터 일관성 확보)
        uint32_t count = pulseCount;
        pulseCount = 0;          // 펄스 수 초기화
        interrupts();            // 인터럽트 재개

        flowRate = (count / PULSES_PER_LITER) * 1000.0;  // L/s → mL/s 변환
        totalMilliLiters += flowRate;                    // 누적 mL 업데이트
        lastUpdateTime = currentTime;
    }
}

// ✅ 현재 유량 반환 함수 (mL/s)
float getFlowRate() {
    return flowRate;
}

// ✅ 누적 유량 반환 함수 (mL)
float getTotalVolume() {
    return totalMilliLiters;
}

// ✅ 유량 초기화 함수 (다음 주입을 위해)
void resetVolume() {
    totalMilliLiters = 0;
}

bool targetWater() {
    return true;
    // if(totalMilliLiters >= targetVolume){
    // return true;
    // }

}

// ✅ 목표 유량 도달 여부 확인 함수
bool targetWater() {
    return getTotalVolume() >= targetVolume;
}
