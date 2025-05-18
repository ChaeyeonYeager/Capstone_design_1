/* LiquidFeedDetection.cpp */
#include "g_LiquidFeedDetection.h"

#if defined(ARDUINO_ARCH_ESP32)
#include <esp_camera.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// 내부 저장
static const char* sssid;
static const char* spassword;
static const char* surl;

// ESP32-CAM 기본 JPEG 설정
static camera_config_t camCfg = {
  .ledc_channel = LEDC_CHANNEL_0,
  .ledc_timer   = LEDC_TIMER_0,
  .pin_d0       = Y2_GPIO_NUM,
  .pin_d1       = Y3_GPIO_NUM,
  .pin_d2       = Y4_GPIO_NUM,
  .pin_d3       = Y5_GPIO_NUM,
  .pin_d4       = Y6_GPIO_NUM,
  .pin_d5       = Y7_GPIO_NUM,
  .pin_d6       = Y8_GPIO_NUM,
  .pin_d7       = Y9_GPIO_NUM,
  .pin_xclk     = XCLK_GPIO_NUM,
  .pin_pclk     = PCLK_GPIO_NUM,
  .pin_vsync    = VSYNC_GPIO_NUM,
  .pin_href     = HREF_GPIO_NUM,
  .pin_sscb_sda = SIOD_GPIO_NUM,
  .pin_sscb_scl = SIOC_GPIO_NUM,
  .pin_pwdn     = PWDN_GPIO_NUM,
  .pin_reset    = RESET_GPIO_NUM,
  .xclk_freq_hz = 20000000,
  .pixel_format = PIXFORMAT_JPEG,
  .frame_size   = FRAMESIZE_VGA,
  .jpeg_quality = 12,
  .fb_count     = 1
};

void initLiquidFeedDetection(const char* wifi_ssid,
                             const char* wifi_password,
                             const char* server_url) {
  sssid = wifi_ssid;
  spassword = wifi_password;
  surl = server_url;

  // WiFi 연결
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print('.');
  }
  Serial.println(" connected");

  // 카메라 초기화
  esp_err_t err = esp_camera_init(&camCfg);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed: 0x%x\n", err);
  }
}

float detectLiquidFeedRatio() {
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) return -1.0f;

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi lost");
    esp_camera_fb_return(fb);
    return -1.0f;
  }

  HTTPClient http;
  http.begin(surl);
  http.addHeader("Content-Type", "image/jpeg");
  int len = http.POST(fb->buf, fb->len);
  String payload;
  if (len > 0) {
    payload = http.getString();
  } else {
    Serial.printf("HTTP POST failed: %s\n", http.errorToString(len).c_str());
  }
  http.end();
  esp_camera_fb_return(fb);

  // JSON 파싱
  StaticJsonDocument<200> doc;
  auto errJ = deserializeJson(doc, payload);
  if (errJ) {
    Serial.println("JSON parse error");
    return -1.0f;
  }
  return doc["remain_ratio"];
}
#endif // ARDUINO_ARCH_ESP32