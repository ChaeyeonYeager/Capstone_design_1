// globals.h
#ifndef GLOBALS_H
#define GLOBALS_H

#include <HX711.h>

// 에너지 계산용 전역 변수
extern float RER;             // 기초 에너지
extern float DER;             // 하루 필요 에너지
extern float portionGrams;    // 1회 급여량 (g)

// 로드셀 측정용 객체
extern HX711 hx711_calc;

// 상태 플래그
extern bool isGrinding;
extern bool feeding_done;
extern bool isProcessDone;

#endif // GLOBALS_H