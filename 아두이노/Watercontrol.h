#ifndef WATER_CONTROL_H
#define WATER_CONTROL_H

// 펌프 및 유량 센서 관련 핀 설정
const int flowSensorPin = 2;    // 유량 센서 핀
const int relayPin = 3; // 펌프 제어 핀
const int targetPulseCount = 515;   // 목표 펄스 수 (100mL 기준)

// ✅ 외부 참조 변수 선언 (상태 제어용)
extern unsigned int pulseCount;  // 현재 펄스 카운트
extern bool isSoaking;                        // 불림 상태 플래그
extern bool isProcessDone;                    // 프로세스 완료 플래그

// 초기화 및 동작 함수
/**
 * 시스템 핀 모드 및 시리얼 초기화
 */
void initWaterSystem();
/**
 *  물 투입 및 불림 대기 통합 실행 (1회만 수행)
 */
void runWaterProcess();

/**
 *  유량 센서 인터럽트용 펄스 카운트 증가 함수
 */
void updatePulseCount();

/**
    불림 완료 여부 반환
    true: 완료 / false: 미완료
 */
bool isSoakingDone();
#endif
// - runWaterProcess(): 펌프 제어 + 불림 통합 처리
// - delay 시간 10분 → 30분으로 연장 (1800000ms)
