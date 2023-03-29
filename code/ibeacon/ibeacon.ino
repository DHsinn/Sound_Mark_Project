#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "BLEDevice.h"
#include "BLEUtils.h"
#include "BLEServer.h"
#include "BLEBeacon.h"

BLEAdvertising *pAdvertising;   // BLE Advertisement type
#define BEACON_UUID "b67b46cc-7009-4d7f-84fa-5ea76b78051d" // UUID 1 128-Bit (may use linux tool uuidgen or random numbers via https://www.uuidgenerator.net/)

// 장치의 MAC ADDRESS를 출력 하는 함수
void printDeviceAddress()
{
  const uint8_t* point = esp_bt_dev_get_address();
  for (int i = 0; i < 6; i++)
  {
    char str[3];
    sprintf(str, "%02X", (int)point[i]);
    Serial.print(str);
    if (i < 5)
    {
      Serial.print(":");
    }
  }
}

void setup()
{
  Serial.begin(115200);

  BLEDevice::init("IBeacon Test"); // BLE을 생성

  printDeviceAddress(); // 장치의 MAC ADDRESS를 출력
  Serial.println("");

  // Create the BLE Server
  BLEServer *pServer = BLEDevice::createServer();
  // BLE ADVERTISING
  pAdvertising = BLEDevice::getAdvertising();
  BLEDevice::startAdvertising();

  // 비콘 설정
  BLEBeacon oBeacon = BLEBeacon();
  oBeacon.setManufacturerId(0x4C00); // 애플의 제조사 고유번호 입니다. fake Apple 0x004C LSB (ENDIAN_CHANGE_U16!)
  oBeacon.setProximityUUID(BLEUUID(BEACON_UUID)); // UUID를 위에 정의한걸로 설정

  Serial.println(BLEUUID(BEACON_UUID).toString().c_str()); // UUID를 시링러 모니터에 출력
  
  // MAJOR 및 MINOR 값 설정
  oBeacon.setMajor(20);
  oBeacon.setMinor(30);
  BLEAdvertisementData oAdvertisementData = BLEAdvertisementData();
  BLEAdvertisementData oScanResponseData = BLEAdvertisementData();

  oAdvertisementData.setFlags(0x04); // BR_EDR_NOT_SUPPORTED 0x04

  std::string strServiceData = "";

  strServiceData += (char)26;     // Len
  strServiceData += (char)0xFF;   // Type
  strServiceData += oBeacon.getData();

  oAdvertisementData.addData(strServiceData);

  pAdvertising->setAdvertisementData(oAdvertisementData);
  pAdvertising->setScanResponseData(oScanResponseData);
  // Start advertising
  pAdvertising->start();
  Serial.println("Advertizing started...");
}

void loop()
{
}
