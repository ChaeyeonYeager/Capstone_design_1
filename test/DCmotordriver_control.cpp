// 핀 정의
const int IN1 = 8;
const int IN2 = 9;
const int ENA = 10;  // PWM 가능 핀

void setup() {
  // 핀 모드 설정
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENA, OUTPUT);

  // 초기에는 모터 정지 상태로 설정
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, 0);  // PWM 0% = OFF
}

void loop() {
  // 정방향 회전 설정
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, 180);  // PWM 속도 설정 (0~255, 여기서는 180으로 중간 속도)

  delay(3000); // 3초간 작동

  // 모터 정지
  analogWrite(ENA, 0);
  delay(5000); // 5초간 정지
}
