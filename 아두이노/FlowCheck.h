#ifndef FLOWSENSOR_H
#define FLOWSENSOR_H

#include <Arduino.h>

// 유량계 센서
class FlowSensor {
public:
    FlowSensor(uint8_t interruptPin, float pulsesPerLiter = 450.0); // 인터럽트 핀 번호 설정, 펄스 당 리터 수 => 보통 450번번인데 이건 보면서 수정해야할수도??
    void begin();
    void update(); // 주기적으로 호출 후 유량 계산
    float getFlowRate(); // 초당 흐른 물 양 (mL/s) 반환
    float getTotalVolume(); // 흐른 전체 물의 양 (mL) 반환
    void resetVolume(); // 누적된 물 양 0으로 초기화

    void pulseISR(); // 인터럽트에서 호출할 함수 => 펄스가 발생할 때마다 호출되는 함수
    bool targetWater();

private:
    uint8_t pin; // 연결 된 핀 번호 
    volatile uint32_t pulseCount; // 받은 펄스 수 (인터럽트로 증가)
    float pulsesPerLiter; // 리터당 펄스 수 (약 450)
    unsigned long lastUpdateTime; // 마지막으로 유량을 계산한 시간
    float flowRate; // 초당 흐르는 유랑 (mL/s)
    float totalMilliLiters; // 전체 누적 유랑 (mL)
};

#endif
