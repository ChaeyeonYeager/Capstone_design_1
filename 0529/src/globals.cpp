// globals.cpp
#include "globals.h"

// 전역 변수 정의 및 초기화
float RER = 0.0;
float DER = 0.0;
float portionGrams = 0.0;

// 로드셀 측정용 객체 인스턴스화
HX711 hx711_calc;

// 상태 플래그 초기화
bool isGrinding = false;
bool feeding_done = false;
bool isProcessDone = false;