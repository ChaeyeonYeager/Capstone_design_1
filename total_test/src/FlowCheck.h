#ifndef FLOW_CHECK_H
#define FLOW_CHECK_H

// 유량 측정 및 펌프 제어를 위한 인터페이스 제공

// ===== 센서 초기화 =====
void initFlowSensor();            // 유량 센서 초기화 (핀/인터럽트 설정)

// ===== 유량 측정 =====
void flowUpdate();                // 1초마다 호출: 유량 계산 및 누적
float getFlowRate();              // 현재 유량 반환 (mL/s)
float getTotalVolume();           // 누적 유량 반환 (mL)
void resetVolume();               // 누적 유량 초기화

// ===== 목표 유량 제어 =====
void setTargetVolume(float mL);   // 목표 주입량 설정 (mL 단위)
bool targetWater();               // 목표량 도달 여부 확인

#endif

// - setTargetVolume(float mL) 함수 추가 → 외부에서 주입 목표량 설정 가능
// - targetWater() 로직 수정 → 누적 유량 기준 비교
// - flowRate 및 totalVolume 계산 안정화 (1초 간격)
