// b_feeder.h
#ifndef B_FEEDER_H
#define B_FEEDER_H

#include <Arduino.h>

/**
 * 피더 하드웨어 초기화
 *  - HX711 로드셀, 서보모터 세팅
 *  - EEPROM에서 이전에 저장된 컨테이너(사료통) 무게 로드
 */
void initFeeder();

/**
 * 컨테이너(사료통) 무게 보정 및 EEPROM에 저장
 *  1) 아무것도 올리지 않은 상태에서 tare
 *  2) 사료통만 올리고 사용자 입력 대기 후 무게 읽어 저장
 */
void calibrateFeeder();

/**
 * 지정된 g 만큼 사료를 투입하고 서보모터 닫기
 * @param targetGrams 투입할 사료량 (g)
 */
void feedFoodProcess(float targetGrams);

#endif // B_FEEDER_H
