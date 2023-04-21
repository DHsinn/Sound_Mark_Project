#include "BLEDevice.h"
#include "BLEUtils.h"
#include "BLEScan.h"
#include "BLEAdvertisedDevice.h"
#include "BLEBeacon.h"

// iBeacon UUID
#define BEACON_UUID "b67b46cc-7009-4d7f-84fa-5ea76b78051d"
#define TEST_BEACON_UUID "4e05b46d-916b-44ff-9896-38f18f4a12b5"

// 스캔 주기 및 스캔 시간
#define SCAN_PERIOD 5
#define SCAN_INTERVAL 100

//스피커 연결 핀번호 (GIOP 번호)
#define speakerpin 23

//오디오
#include <SoftwareSerial.h>
#include <MP3Player_KT403A.h>
SoftwareSerial mp3(17, 16); //TX, RX  (GIOP 번호)

//절전모드
#define SLEEP_DURATION 10 // 절전 모드 시간 (초)
#define NO_COMMAND_DURATION 60 // 명령 없는 경우 절전 모드로 돌아가는 시간 (초)

BLEAdvertising *pAdvertising;   //송출 포인터설정
BLEScan* pBLEScan;   //스캔포인터설정
bool isBeaconDetected = false;

//비콘 manufacturerr data 로 감지하는 클래스
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      Serial.printf("Advertised Device: %s \n", advertisedDevice.toString().c_str());

      // iBeacon 감지 (manufacturerdata가 있는지 확인하고 iBeacon 패킷 구성에 맞는 manufacturerData인지 확인하기)
      if (advertisedDevice.haveManufacturerData() && advertisedDevice.getManufacturerData().length() == 25 &&
          advertisedDevice.getManufacturerData()[0] == 0x4c && advertisedDevice.getManufacturerData()[1] == 0x00 &&
          advertisedDevice.getManufacturerData()[2] == 0x02 &&  advertisedDevice.getManufacturerData()[3] == 0x15) {
        
        Serial.println("iBeacon detected!");
        
        std::string payload = advertisedDevice.getManufacturerData();
        uint16_t major = payload[20] << 8 | payload[21];
        uint16_t minor = payload[22] << 8 | payload[23];
        Serial.printf("Major: %d, Minor: %d\n", major, minor);
        
        // Major와 Minor 값에 따라서 처리할 작업을 구현
        if (major == 0 && minor == 10) {
          Serial.println("Processing command for detected iBeacon...");
          // 원하는 명령 처리
          isBeaconDetected = true;
        }
      }
    }
};

// 절전 모드로 진입
void sleepMode() {
    Serial.println("Entering sleep mode...");
    BLEDevice::deinit();
    esp_deep_sleep(SLEEP_DURATION * 1000000);
}

void setup() {
  // Serial.begin(115200);

  //--------------------비콘구성---------------------
  BLEDevice::init("RIVO_iBeacon");
  BLEServer *pServer = BLEDevice::createServer();

  BLEBeacon oBeacon = BLEBeacon();
  oBeacon.setManufacturerId(0x4c00);   //company ID
  oBeacon.setProximityUUID(BLEUUID("8ec76ea3-6668-48da-9866-75be8bc86f4d"));    //UUID
  oBeacon.setMajor(100);    //Major
  oBeacon.setMinor(1);     //Minor

  // 외부로 송출할 데이터 변수 생성하고 변수에 비콘 데이터 담아서 송출
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
  //------------------------------------------------
  //---------------------mp3 구성-------------------
    mp3.begin(9600);
    Serial.begin(9600); 
    delay(100);
    
    SelectPlayerDevice(0x02);       // Select SD card as the player device. 내 디바이스에 있는 SD카드에서 파일 가져오기
    SetVolume(0x0E);        //볼륨설정하기 최대 0x1E
  //------------------------------------------------

  //------------------비콘검색-----------------------
  //BLEDevice::init(""); // BLE을 생성
  pBLEScan = BLEDevice::getScan(); // 새로운 스캔 객체 생성
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(SCAN_INTERVAL);
  pBLEScan->setWindow(SCAN_INTERVAL - 1);
  pBLEScan->setActiveScan(true); // 활성화된 스캔은 더 많은 전력을 사용하지만 더 빠르게 결과를 얻을 수 있습니다.
  //-------------------------------------------------
}

void loop() {
  BLEScanResults scanResults = pBLEScan->start(SCAN_PERIOD, false);
  char rec = 0;
  if (isBeaconDetected) {
    // iBeacon을 감지한 경우, 원하는 명령 수행
    Serial.println("iBeacon detected, performing task...");
    while(1){
      rec = Serial.read();
      if(rec='1'||rec=='2'||rec=='3'||rec=='4'||rec=='5'||rec=='6'||rec=='7'||rec=='8'||rec=='9'){
        Serial.println( rec );
      }
      switch (rec){
        case '1':     //노래 재생
            SpecifyMusicPlay(1);
            Serial.println("Specify the music index to play");
            break;
        case '2':     //노래 일시정지
            PlayPause();
            Serial.println("Pause the MP3 player");
            break;
        case '3':     //노래가 정지된 대부터 다시 재생
            PlayResume();
            Serial.println("Resume the MP3 player");
            break;
        case '4':    //다음 노래로
            PlayNext();
            Serial.println("Play the next song");
            break;
        case '5':    // 이전 노래로
            PlayPrevious();
            Serial.println("Play the previous song");
            break;
        case '6':    //노래들을 전체반복 재생한다
            PlayLoop();
            Serial.println("Play loop for all the songs");
            break;
        case '7':  //볼륨을 높인다
            IncreaseVolume();
            Serial.println("Increase volume");
            break;
        case '8':    //볼륨을 낮춘다
            DecreaseVolume();
            Serial.println("Decrease volume");
            break;
        case '9':   //mp3노래가 멈추고 연결된 스피커에서 부저가 울린다.
            tone(speakerpin,500,500);
            PlayPause();
            Serial.println("booooo!! Pause the MP3 player");
            break;
        default:
            break;
      }
      if (millis() >= NO_COMMAND_DURATION * 1000) { // 1분간 명령이 없으면 절전 모드로 진입
        sleepMode();
      }
      delay(1000);
    }
    isBeaconDetected = false;
  } else {
    Serial.println("iBeacon not detected");
    sleepMode();
  }
  pBLEScan->clearResults(); // 결과 버퍼를 지워서 메모리를 해제합니다.
  //delay(10000);
}

// 딥슬립에서 깨어났을 때 실행되는 함수
extern "C" void app_mam() {
    setup();
    loop();
}







/*
//비콘 service UUID로 감지하는 클래스
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      Serial.printf("Advertised Device: %s \n", advertisedDevice.toString().c_str());
      // iBeacon 감지
      Serial.println(advertisedDevice.haveServiceUUID());      //serviceuuid가 있는지 확인
      Serial.println(advertisedDevice.isAdvertisingService(BLEUUID(TEST_BEACON_UUID)));   //그 uuid가 일치하는지 확인
      if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(BLEUUID(TEST_BEACON_UUID))) {
        Serial.println("iBeacon` detected!");
        std::string payload = advertisedDevice.getServiceData();
        uint16_t major = payload[18] << 8 | payload[19];
        uint16_t minor = payload[20] << 8 | payload[21];
        Serial.printf("Major: %d, Minor: %d\n", major, minor);
        // Major와 Minor 값에 따라서 처리할 작업을 구현
        if (major == 0 && minor == 10) {
          Serial.println("Processing command for detected iBeacon...");
          bool isBeaconDetected = true;
        }
      }
    }
};
*/