const int relayPin = 8;  // 릴레이 S핀 → 아두이노 D8 연결

void setup() {
  pinMode(relayPin, OUTPUT);           // 릴레이 제어 핀 설정
  digitalWrite(relayPin, LOW);        // 초기 릴레이 OFF 상태
  Serial.begin(9600);                  // 시리얼 모니터 시작
}

void loop() {
  Serial.println("펌프 ON");
  digitalWrite(relayPin, HIGH);         // 릴레이 ON 
  delay(5000);                         // 5초간 물 주입

  Serial.println("펌프 OFF");
  digitalWrite(relayPin, LOW);        // 릴레이 OFF
  delay(5000);                         // 5초간 멈춤
}
