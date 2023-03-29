#include <SoftwareSerial.h>
#define SAFE_RX 8
#define SAFE_TX 7

SoftwareSerial SAFE(SAFE_RX, SAFE_TX); // RX핀(7번)은 HM10의 TX에 연결
// TX핀(8번)은 HM10의 RX에 연결
void setup() {
  Serial.begin(9600);
  SAFE.begin(9600);
}
void loop() {
  if (SAFE.available()) {
    Serial.write(SAFE.read());
  }
  SAFE.write("Uwait\n");
  delay(1000);
  if (Serial.available()) {
    SAFE.write(Serial.read());
  }
}
