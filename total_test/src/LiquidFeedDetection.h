#ifndef LIQUIDFEEDDETECTION_H
#define LIQUIDFEEDDETECTION_H

#if defined(ARDUINO_ARCH_ESP32)

#include <Arduino.h>
#include <esp_camera.h>
#include <WiFi.h>
#include <HTTPClient.h>

/**
 * @brief ESP32-CAM에서 유동식 잔여량을 서버로 분석 요청하는 함수 모음
 */

/**
 * @brief WiFi 및 카메라 초기화 (ESP32-CAM 전용)
 * @param wifi_ssid     WiFi SSID
 * @param wifi_password WiFi 비밀번호
 * @param server_url    분석 서버 URL (http://IP:포트/엔드포인트)
 */
void initLiquidFeedDetection(const char* wifi_ssid,
                             const char* wifi_password,
                             const char* server_url);

/**
 * @brief 카메라 이미지를 서버로 전송 후 JSON에서 remain_ratio 추출
 * @return float 남은 유동식 면적 비율(0.0~1.0), 실패 시 음수
 */
float detectLiquidFeedRatio();

#else

#warning "LiquidFeedDetection is only supported on ESP32-CAM"

inline void initLiquidFeedDetection(const char*, const char*, const char*) {}
inline float detectLiquidFeedRatio() { return -1.0f; }

#endif // ARDUINO_ARCH_ESP32

#endif // LIQUIDFEEDDETECTION_H