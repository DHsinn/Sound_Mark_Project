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





/*
BLEScan* pBLEScan;   // BLE 스캔을 위한 BLEScan 객체를 선언
bool isDeviceFound = false;   //디바이스가 검색되었는지 여부를 저장하는 변수를 선언하고 초기값을 false로 설정
String targetAddress = "5C:CB:99:DF:56:98"; // 등록된 사용자의 MAC 주소를 설정합니다.

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    std::String address = advertisedDevice.getAddress().toString();
    if (address == targetAddress) {
      // 등록된 사용자의 블루투스가 검색됐습니다.
      Serial.println("Detected user's Bluetooth device!");
      isDeviceFound = true;  // 검색 되어서 true 로 변경
    }
  }
};


void setup() { //블루투스 모듈을 초기화하고 iBeacon 객체를 생성
  Serial.begin(115200);
  BLEDevice::init("RIVO iBeacon");  //BLE 라이브러리를 초기화 "BLE 프로파일 이름"
  BLEServer *pServer = BLEDevice::createServer();
  BLEBeacon oBeacon = BLEBeacon();
  oBeacon.setManufacturerId(0x4C00);  // Apple의 회사 식별자를 설정
  oBeacon.setProximityUUID(BLEUUID("00000000-0000-0000-0000-000000000000")); //  iBeacon의 UUID 값을 설정
  oBeacon.setMajor(0x0001);  //  major, minor 값을 설정
  oBeacon.setMinor(0x0001); 
  BLEAdvertisementData oAdvertisementData = BLEAdvertisementData();  //객체를 생성
  oAdvertisementData.setFlags(0x04); // setFlags() 함수를 사용하여 BR/EDR이 지원되지 않는 것을 설정
  oAdvertisementData.setManufacturerData(oBeacon.getData());    //iBeacon 데이터를 설정
  pAdvertising = BLEDevice::getAdvertising();  //광고 객체를 가져온 후
  pAdvertising->setAdvertisementData(oAdvertisementData);  //setAdvertisementData() 함수와 setScanResponseData() 함수를 사용하여 광고 데이터와 응답 데이터를 설정
  pAdvertising->setScanResponseData(oAdvertisementData);
  pAdvertising->setMinPreferred(0x06);  // 연결 문제를 해결하기 위한 최소 값 설정
  pAdvertising->setMinPreferred(0x12);  // 연결 문제를 해결하기 위한 최소 값 설정

  Serial.println("Starting BLE scan...");
  pBLEScan = BLEDevice::getScan();  //BLE 스캔을 위한 BLEScan 객체를 가져옴
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());  //BLE 스캔 결과를 처리할 콜백 함수를 등록
  pBLEScan->setActiveScan(true);  //활성 스캔 모드를 설정
  pBLEScan->setInterval(100);  //스캔 간격을 100ms로 설정
  pBLEScan->setWindow(99);  //스캔 창 크기를 99ms로 설정
}

void loop() {
  pAdvertising->start();
  if (!isDeviceFound) {
    BLEScanResults foundDevices = pBLEScan->start(1);  //BLE 스캔을 시작 인자는 스캔 시간(초) 스캔 결과는 BLEScanResults 객체로 반환
    pBLEScan->clearResults(); // 이전 검색 결과 지우기
    pAdvertising->stop();
    delay(5000);  //5초 절전모드
  } 
  else {
    Serial.println("Device detected!");
    // 디바이스가 검색되면 추가적인 처리구현
    while (true) {   //신호대기 유지
      // if (Serial.available()) {
      //   char input = Serial.read();
      //   if (input == '1') {
      //     // "1"이 입력되면 소리를 내도록 구현합니다.
      Serial.println("Beep!");
      tone(23, 5000);
      delay(1000);
      noTone(8);
    }
  }
}*/