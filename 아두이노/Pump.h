#ifndef PUMP_H
#define PUMP_H

// 펌프 제어 모듈 인터페이스

void initPump();       // 펌프 및 유량 센서 초기화
void pumpUpdate();     // 유량 체크 및 펌프 자동 OFF
bool isPumpOn();       // 현재 펌프 작동 상태 반환

#endif

// - 펌프 ON/OFF 로직 완성 (유량 목표 도달 시 자동 OFF)
// - flowUpdate() 연동으로 1초 단위 유량 갱신
// - isPumpOn() 상태 반환 추가
