#ifndef PINNUM_H

// 1) 사료량 계산.. 패스

// 2) 사료 떨어트리기
// (로드셀)
#define doutPin   2
#define sckPin    3
//  (서보)
#define servoPin  9

// 3) 1차 + 2차 물 주입

// 릴레이
#define relayPin 10
// 유량
#define flowSensorPin 11

// 4) 유동식 제조
#define RPWM  5     // PWM 신호 (정방향 회전용)
#define LPWM  6     // 반대 방향 (LOW 유지)
#define R_EN  7
#define L_EN  8


// 5) 사료 잔여량 체크

// (초음파 1)
#define TRIG1 A0
#define ECHO1 A1
// (초음파 2)
#define TRIG2 A2
#define ECHO2 A3
// LED
#define LED_PIN 13

#endif
