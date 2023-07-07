#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <BLEAddress.h>

#define RSSI_LIMIT -70
#define BEACON_COUNT 1

String addr_list[BEACON_COUNT];
int check_list[BEACON_COUNT];

int scanTime = 5; //In seconds
BLEScan* pBLEScan;
/*
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks
{
    void onResult(BLEAdvertisedDevice advertisedDevice)
    {
      Serial.print("FOUND : ");
      Serial.print(advertisedDevice.getName().c_str());
      Serial.print(" , ");
      Serial.print(advertisedDevice.getAddress().toString().c_str());
      Serial.println("");
      for ( int i = 0; i < BEACON_COUNT; i++)
      {
        BLEAddress addr(advertisedDevice.getAddress());
        if ( addr_list[i] == addr.toString().c_str() )
        {
          if ( advertisedDevice.getRSSI() > RSSI_LIMIT )
          {
            check_list[i] = 1;
          }
          Serial.print(advertisedDevice.getName().c_str());
          Serial.print(" ");
          Serial.print(addr.toString().c_str());
          Serial.print(" ");
          Serial.print(advertisedDevice.getRSSI());
          Serial.println("");
        }
      }
    }
};
*/
void setup()
{
  pinMode(14, OUTPUT);
  Serial.begin(115200);
  Serial.println("Scanning...");

  // 비콘 장치의 MAC ADDRESS를 아래에 적으세요
  addr_list[0] = "C2:02:0F:00:05:18";
//  addr_list[0] = "ff:ff:00:08:a9:ff";
//  addr_list[1] = "ff:ff:c0:39:73:85";
//  addr_list[2] = "ff:ff:c0:3a:2e:b0";

  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan(); //create new scan
  //pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);  // less or equal setInterval value
}

void loop()
{
  for ( int i = 0; i < BEACON_COUNT; i++)
  {
    check_list[i] = 0;
  }
  BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
  pBLEScan->clearResults();
  Serial.println("########## RESULT ##########");

  int check_count = 0;
  for ( int i = 0; i < BEACON_COUNT; i++)
  {
    Serial.print(i);
    Serial.print(" ");
    if ( check_list[i] == 0 )
    {
      Serial.print("NG");
    }
    else
    {
      Serial.print("OK");
      check_count++;
    }
    Serial.println("");
  }

  unsigned long start_millis = millis();
  while ( ( millis() - start_millis ) < 10000 )
  {
    if ( check_count != BEACON_COUNT )
    {
      digitalWrite(14, HIGH);
      delay(500);
      digitalWrite(14, LOW);
      delay(500);
    }
    else
    {
      digitalWrite(14, LOW);
      delay(500);
    }
  }
}
