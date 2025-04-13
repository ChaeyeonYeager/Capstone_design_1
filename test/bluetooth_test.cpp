#include <SoftwareSerial.h>

// ===================== 핀 설정 =====================
#define BT_RX 10  // 아두이노가 블루투스로부터 받는 핀
#define BT_TX 11  // 아두이노가 블루투스로 보내는 핀

// ===================== 블루투스 객체 생성 =====================
SoftwareSerial BT(BT_RX, BT_TX);  // HC-06 연결

void setup() {
  Serial.begin(9600);    // 시리얼 모니터
  BT.begin(9600);        // HC-06 기본 통신 속도 (9600)

  Serial.println("📡 블루투스 테스트 시작! HC-06과 연결되었는지 확인하세요.");
  Serial.println("▶ 시리얼 창에 입력하면 블루투스로 전송됩니다.");
}

void loop() {
  // 블루투스 → 시리얼 모니터 출력
  if (BT.available()) {
    String input = BT.readStringUntil('\n');
    Serial.println("📥 블루투스로 받은 데이터: " + input);
  }

  // 시리얼 입력 → 블루투스로 전송
  if (Serial.available()) {
    String msg = Serial.readStringUntil('\n');
    BT.println("📤 아두이노에서 보낸 데이터: " + msg);
  }
}
