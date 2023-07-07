#include <SoftwareSerial.h>

SoftwareSerial BTSerial(3, 1); // SoftwareSerial(RX, TX)

void setup()
{
  Serial.begin(115200);
  Serial.println("Hello!");
  // set the data rate for the BT port
  BTSerial.begin(115200);
}

void loop(){
  while (BTSerial.available()){ // if BT sends something
    byte data = BTSerial.read();
    Serial.write(data); // write it to serial(serial monitor)
  }
  while (Serial.available()){ // if Serial has input(from serial monitor)
    byte data = Serial.read();
    BTSerial.write(data); // write it to BT
  }
}
