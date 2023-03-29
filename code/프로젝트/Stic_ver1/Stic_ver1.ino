#include <SoftwareSerial.h>
#define SIGN_RX 8
#define SIGN_TX 7
#define SAFE_RX 2
#define SAFE_TX 3
#define VIBRATION 11
#define trigPin 13
#define echoPin 12

int blue_distacne;//블루투스로 측정한거리
int echo_distance;
int cnt = 0;
SoftwareSerial SIGN(SIGN_RX, SIGN_TX); // 신호등과 통신할 블루투수 모듈
SoftwareSerial SAFE(SAFE_RX, SAFE_TX);
void setup() {
  Serial.begin(9600);
  SIGN.begin(9600); //  신호등과 지팡이 송수신준비
  
  pinMode(VIBRATION,OUTPUT);
  pinMode(trigPin,OUTPUT);
  pinMode(echoPin,INPUT);
}

//초음파센서 거리구하는 함수 가까우면 진동발생
void How_long_echo(){
  //trigger 핀으로 10us의 펄스를 발생
  int echo_distance;//초음파센서로 측정한거리
  digitalWrite(trigPin,HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin,LOW);
  echo_distance = pulseIn(echoPin,HIGH)/58;
  
  if(echo_distance <= 50){
    analogWrite(VIBRATION,255-(echo_distance*4));
   // Serial.print("how_distance: ");
   // Serial.println(echo_distance);
    
  }
  else{
    analogWrite(VIBRATION,0);
  }
}

void loop() {
  
    SIGN.begin(9600);
    if (SIGN.available()) {
      Serial.write(SIGN.read());
    }
  
    if (Serial.available()) {
      SIGN.write(Serial.read());
    }     
  /*
  SAFE.begin(9600);
      if (SAFE.available()) {
      Serial.write(SAFE.read());
    }
    
    if (Serial.available()) {
      SAFE.write(Serial.read());
    }
    */
  
  /*
    //How_long_echo();// 초음파센서 측정 50센티보다 가까우면 위치별 진동을 다르게줌
    if (SIGN.available()) {
      Serial.write(SIGN.read());
    }
  
    if (Serial.available()) {
      SIGN.write(Serial.read());
    }     
  

  
    //How_long_echo();// 초음파센서 측정 50센티보다 가까우면 위치별 진동을 다르게줌
    if (SAFE.available()) {
      Serial.write(SAFE.read());
    }
    
    if (Serial.available()) {
      SAFE.write(Serial.read());
    }
    */
}
