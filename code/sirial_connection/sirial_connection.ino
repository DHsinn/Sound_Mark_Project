#include <HardwareSerial.h>

void setup() {
  // 시리얼 통신 초기화
  Serial.begin(115200);
}

void loop() {
  // 입력한 데이터가 있을 경우
  if (Serial.available()) {
    // 시리얼 포트로부터 데이터를 읽어서 변수에 저장
    char incomingData = Serial.read();

    // 읽은 데이터를 시리얼 포트로 다시 출력
    Serial.write(incomingData);
  }
}