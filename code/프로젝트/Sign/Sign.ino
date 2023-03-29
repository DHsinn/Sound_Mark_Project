#include <SoftwareSerial.h>
#define SIGN_RX 8
#define SIGN_TX 7

SoftwareSerial SIGN(SIGN_RX, SIGN_TX); // RX핀(7번)은 HM10의 TX에 연결
// TX핀(8번)은 HM10의 RX에 연결
void setup() {
  Serial.begin(9600);
  SIGN.begin(9600);
}
void loop() {
  if (SIGN.available()) {
    Serial.write(SIGN.read());
  }
  SIGN.write("Swait\n");
  delay(1000);
  if (Serial.available()) {
    SIGN.write(Serial.read());
  }
}
