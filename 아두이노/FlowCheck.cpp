#include <Arduino.h>
#include "FlowCheck.h"

#define FLOW_SENSOR_PIN 2         // 핀 고정
#define PULSES_PER_LITER 450.0    // 펄스 수 고정

volatile uint32_t pulseCount;
unsigned long lastUpdateTime;
float flowRate;
float totalMilliLiters;

void initFlowSensor() {
    pulseCount = 0;
    lastUpdateTime = 0;
    flowRate = 0;
    totalMilliLiters = 0;

    pinMode(FLOW_SENSOR_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(FLOW_SENSOR_PIN), pulseISR, RISING);
    lastUpdateTime = millis();
}

void pulseISR() {
    pulseCount++;
}

void flowUpdate() {
    unsigned long currentTime = millis();
    unsigned long deltaTime = currentTime - lastUpdateTime;

    if (deltaTime >= 1000) {
        noInterrupts();
        uint32_t count = pulseCount;
        pulseCount = 0;
        interrupts();

        flowRate = (count / PULSES_PER_LITER) * 1000.0;
        totalMilliLiters += flowRate;
        lastUpdateTime = currentTime;
    }
}

float getFlowRate() {
    return flowRate;
}

float getTotalVolume() {
    return totalMilliLiters;
}

void resetVolume() {
    totalMilliLiters = 0;
}

bool targetWater() {
    return true;
}
