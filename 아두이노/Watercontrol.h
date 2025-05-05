#ifndef WATER_CONTROL_H
#define WATER_CONTROL_H

// 펌프 및 유량 센서 관련 핀 설정
const int pumpPin = 7;             // 펌프 제어용 릴레이 핀
const int flowSensorPin = 3;       // 유량 센서 신호 핀

const int targetPulseCount = 45;   // 목표 펄스 수 (100mL 기준)

// 초기화 및 동작 함수
void initWaterSystem();            // 핀 모드 및 인터럽트 초기화
void runWaterProcess();            // 물 투입 + 불림 통합 프로세스
void countFlowPulse();             // 인터럽트용 펄스 증가 함수
bool isSoakingDone();              // 불림 완료 여부 반환

#endif


// - runWaterProcess(): 펌프 제어 + 불림 통합 처리
// - delay 시간 10분 → 30분으로 연장 (1800000ms)
