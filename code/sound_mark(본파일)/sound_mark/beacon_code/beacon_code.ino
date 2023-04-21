#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEBeacon.h>
#include <BLEServer.h>

BLEAdvertising *pAdvertising;

void setup() {
  BLEDevice::init("iBeacon");
  BLEServer *pServer = BLEDevice::createServer();

  BLEBeacon oBeacon = BLEBeacon();
  oBeacon.setManufacturerId(0x4c00);
  oBeacon.setProximityUUID(BLEUUID("8ec76ea3-6668-48da-9866-75be8bc86f4d"));
  oBeacon.setMajor(100);
  oBeacon.setMinor(1);

  // 외부로 송출할 데이터 변수 생성하고 비콘 데이터 담아서 송출
  std::string strServiceData = "";
  strServiceData += (char)26;     // 데이터 길이가 총 26인데 이중 데이터가 25, 데이터 유형 1바이트
  strServiceData += (char)0xFF;   // 데이터 유형 0xFF  
  strServiceData += oBeacon.getData();

  BLEAdvertisementData oAdvertisementData = BLEAdvertisementData();
  oAdvertisementData.setFlags(0x04); // BR_EDR_NOT_SUPPORTED
  oAdvertisementData.addData(strServiceData);
  
  pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->setAdvertisementData(oAdvertisementData);
  pAdvertising->setScanResponseData(oAdvertisementData);
  pAdvertising->start();
}

void loop() {

}
