#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

BLEScan* pBLEScan;
bool isIBeacon = false;
//String targetUUID = "YOUR_UUID_HERE"; // 검색할 iBeacon의 UUID 값을 설정합니다.
String targetMajor = "0"; // 검색할 iBeacon의 Major 값을 설정합니다.
String targetMinor = "10"; // 검색할 iBeacon의 Minor 값을 설정합니다.

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    if (advertisedDevice.haveManufacturerData()) {
      std::string manufacturerData = advertisedDevice.getManufacturerData();
      if (manufacturerData.length() >= 25) {
        // iBeacon 광고 데이터를 파싱합니다.
        uint16_t companyID = (uint16_t)manufacturerData[0] << 8 | (uint16_t)manufacturerData[1];
        if (companyID == 0x4C && manufacturerData[2] == 0x00 && manufacturerData[3] == 0x02 && manufacturerData[4] == 0x15) {
          // iBeacon 데이터를 찾았습니다.
          //std::string uuid = manufacturerData.substr(4, 16);
          std::string major = manufacturerData.substr(20, 2);
          std::string minor = manufacturerData.substr(22, 2);
          //String strUUID = BLEUtils::buildHexData(uuid.c_str(), uuid.length());
          String strMajor = BLEUtils::buildHexData(major.c_str(), major.length());
          String strMinor = BLEUtils::buildHexData(minor.c_str(), minor.length());
          //Serial.println("Found iBeacon: UUID = " + strUUID + ", Major = " + strMajor + ", Minor = " + strMinor);
          Serial.println("Major = " + strMajor + ", Minor = " + strMinor);
          // 검색할 iBeacon UUID, Major, Minor 값과 일치하는지 확인합니다.
          if (strMajor == targetMajor && strMinor == targetMinor) { //strUUID == targetUUID && 
            isIBeacon = true;
          }
        }
      }
    }
  }
};

void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE scan...");
  BLEDevice::init("RIVO iBeacon");
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);
}

void loop() {
  if (isIBeacon) {
    Serial.println("iBeacon detected!");
    // iBeacon이 검색되면 추가적인 처리를 수행합니다.
    Serial.println("Beep!");
    tone(23, 5000);
    delay(1000);
    noTone(8);
  } else {
    BLEScanResults foundDevices = pBLEScan->start(1);
    pBLEScan->clearResults(); // 이전 검색 결과를 지웁니다.
    delay(1000);
  }
}

