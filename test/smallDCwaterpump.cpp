const int relayPin = 8;  // 릴레이 S 핀 → D8 연결

void setup() {
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW); //초기상태 off
  Serial.begin(9600);
}

void loop() {
  Serial.println("펌프 on");
  digitalWrite(relayPin,HIGH);  //릴레이 on -> 펌프 on
  delay(10000);  //10초 동안 펌프 on

  Serial.println("펌프 off");
  digitalWrite(relayPin,LOW); //릴레이 off -> 펌프 off
  delay(3000);  //3초 동안 펌프 off
}
