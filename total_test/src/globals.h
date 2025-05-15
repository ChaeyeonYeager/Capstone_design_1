#ifndef GLOBALS_H
#define GLOBALS_H

// 📌 전역 보정값 (모든 모듈에서 사용 가능)
extern float calibration_factor;

// 📌 EEPROM 저장/불러오기 함수 선언
void loadCalibrationFromEEPROM();
void saveCalibrationToEEPROM(float f);

#endif
