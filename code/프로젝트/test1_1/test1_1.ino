#include <SoftwareSerial.h>
#define BT_RX 2
#define BT_TX 3
char c;
int tmp=0;
SoftwareSerial HC06(BT_RX, BT_TX); // RX핀(7번)은 HC06의 TX에 연결
// TX핀(8번)은 HC06의 RX에 연결
void setup() {
  Serial.begin(9600);
  HC06.begin(9600);
  pinMode(8,OUTPUT);
  
}
void loop() {
  if (HC06.available()) {
    //Serial.write(HC06.read());
    c = HC06.read();
    Serial.write(c);
    if(c == '1' && tmp == 0){
      Serial.println("실행1");
      digitalWrite(8,HIGH);
      tmp=1;
    }
    else if(c == '1' && tmp == 1){
      Serial.println("실행2");
      digitalWrite(8,LOW);
      tmp=0;
    }
  }
  if (Serial.available()) {
    HC06.write(Serial.read());
  }
}
