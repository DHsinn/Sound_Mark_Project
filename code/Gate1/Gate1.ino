#include "BLEDevice.h"
#include "BLEUtils.h"
#include "BLEScan.h"
#include "BLEAdvertisedDevice.h"
#include "BLEBeacon.h"
#include "esp_sleep.h"

//---------------------------------------------------------------Gate1 코드---------------------------------------------------------------

// iBeacon UUID
#define BEACON_UUID "8ec76ea3-6668-48da-9866-75be8bc86f4d"
#define TEST_BEACON_UUID "39ED98FF-2900-441A-802F-9C398FC199D2"
//#define ManufacturerData "4c00021539ed98ff2900441a802f9c398fc199d2"

// 스캔 주기 및 스캔 시간
//#define SCAN_PERIOD 10  //스캔주기 초
#define SCAN_INTERVAL 100

int previousMajor = 40; // 이전에 체크한 비콘의 Major 값
int previousMinor = 15; // 이전에 체크한 비콘의 Minor 값
const int SCAN_PERIOD = 10000; // 스캔 주기 (ms, 10초)

//오디오
#include <SoftwareSerial.h>
#include <MP3Player_KT403A.h>
SoftwareSerial mp3(17, 16); //TX, RX  (GIOP 번호)

//절전모드
unsigned long lastSignalTime = 0;     //마지막으로 신호가 들어온 거 체크
const int NO_SIGNAL_DURATION = 60 * 1000; // 60초

BLEAdvertising *pAdvertising;   //송출 포인터설정
BLEScan* pBLEScan;   //스캔포인터설정

//비콘 manufacturerdata 로 감지하는 클래스
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  private:
    BLEBeacon oBeacon;
    BLEAdvertisementData oAdvertisementData;

  public:
    MyAdvertisedDeviceCallbacks() {
      // 객체 초기화
      oBeacon = BLEBeacon();
      oAdvertisementData = BLEAdvertisementData();
      pAdvertising = BLEDevice::getAdvertising();
    }

  void onResult(BLEAdvertisedDevice advertisedDevice) {
    //Serial.printf("Advertised Device: %s \n", advertisedDevice.toString().c_str());

    // iBeacon 감지 (manufacturerdata가 있는지 확인하고 iBeacon 패킷 구성에 맞는 manufacturerData인지 확인하기)
    if (advertisedDevice.haveManufacturerData() && advertisedDevice.getManufacturerData().length() == 25 &&
        advertisedDevice.getManufacturerData()[0] == 0x4c && advertisedDevice.getManufacturerData()[1] == 0x00 &&
        advertisedDevice.getManufacturerData()[2] == 0x02 &&  advertisedDevice.getManufacturerData()[3] == 0x15 &&   //여기까지는 아이비콘 기본구성
        advertisedDevice.getManufacturerData()[4] == 0x39 &&  advertisedDevice.getManufacturerData()[5] == 0xed &&   //여기부터 uuid
        advertisedDevice.getManufacturerData()[6] == 0x98 &&  advertisedDevice.getManufacturerData()[7] == 0xff &&
        advertisedDevice.getManufacturerData()[8] == 0x29 &&  advertisedDevice.getManufacturerData()[9] == 0x00 ){
          /*
        advertisedDevice.getManufacturerData()[10] == 0x44 &&  advertisedDevice.getManufacturerData()[11] == 0x1a &&
        advertisedDevice.getManufacturerData()[12] == 0x80 &&  advertisedDevice.getManufacturerData()[13] == 0x2f &&
        advertisedDevice.getManufacturerData()[14] == 0x9c &&  advertisedDevice.getManufacturerData()[15] == 0x39 &&
        advertisedDevice.getManufacturerData()[16] == 0x8f &&  advertisedDevice.getManufacturerData()[17] == 0xc1 &&
        advertisedDevice.getManufacturerData()[18] == 0x99 &&  advertisedDevice.getManufacturerData()[19] == 0xd2 ) { 
        */

      //Serial.println("지정된 iBeacon 탐색됨.");

      //비콘 찾은 후 신호 방출
      oBeacon.setManufacturerId(0x4c00);   //company ID
      oBeacon.setProximityUUID(BLEUUID(BEACON_UUID));    //UUID
      oBeacon.setMajor((2<<8)+3);  // 0000 0001 0000 0003
      oBeacon.setMinor(3);

      oAdvertisementData.setFlags(0x04);
      oAdvertisementData.setManufacturerData(oBeacon.getData());

      pAdvertising->setAdvertisementData(oAdvertisementData);
      pAdvertising->setScanResponseData(oAdvertisementData);
      pAdvertising->start();

      // major, minor 값 뽑아오기
      std::string payload = advertisedDevice.getManufacturerData();
      uint16_t currentMajor = payload[20] << 8 | payload[21];
      uint16_t currentMinor = payload[22] << 8 | payload[23];
      //Serial.printf("Major: %d, Minor: %d\n", currentMajor, currentMinor);

      //if (currentMajor != previousMajor || currentMinor != previousMinor) {
          // 원하는 비콘의 Major, Minor 값이 변경되면 실행할 코드 작성
          // Major, Minor 값을 업데이트
          //previousMajor = currentMajor;
          //previousMinor = currentMinor;

          //Serial.println("비콘의 Major 또는 Minor 값이 변경되었습니다.");

      if (currentMajor == (1<<8) && currentMinor == 3) {
        //Serial.println("주변 스캔신호~!!");
        //Serial.println("주변을 스캔합니다.");
        pBLEScan->start(SCAN_PERIOD, true);
      }

      else if(currentMajor == (3<<8) && currentMinor == 3){ //&& millis() - lastSongPlayTime >= SONG_IGNORE_DURATION

        //Serial.println("노래 재생신호~!!");
        //Serial.println("노래를 재생합니다.");
        //비콘 찾아서 신호 보내주려고 major, minor 값 바꿔서 전송

        oBeacon.setManufacturerId(0x4c00);   //company ID
        oBeacon.setProximityUUID(BLEUUID(BEACON_UUID));    //UUID
        oBeacon.setMajor((4<<8)+3);
        oBeacon.setMinor(3);

        oAdvertisementData.setFlags(0x04);
        oAdvertisementData.setManufacturerData(oBeacon.getData());

        pAdvertising->setAdvertisementData(oAdvertisementData);
        pAdvertising->setScanResponseData(oAdvertisementData);
        pAdvertising->start();

        SpecifyMusicPlay(2);
        }
      }
    }
  };

void setup() {
  //Serial.begin(115200);

  //--------------------비콘구성---------------------
  BLEDevice::init("RIVO_Beacon");
  BLEDevice::startAdvertising();

  //lastSignalTime = millis();  // 타이머 초기화
  //---------------------mp3 구성-------------------
    mp3.begin(9600);
    //delay(100);
    
    SelectPlayerDevice(0x02);       // Select SD card as the player device. 내 디바이스에 있는 SD카드에서 파일 가져오기 
    SetVolume(0x1E);        //볼륨설정하기 최대 0x1E
  //------------------------------------------------

  //------------------비콘검색-----------------------
  pBLEScan = BLEDevice::getScan(); // 새로운 스캔 객체 생성
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(SCAN_INTERVAL);
  pBLEScan->setWindow(SCAN_INTERVAL - 1);
  pBLEScan->setActiveScan(true); // 활성화된 스캔은 더 많은 전력을 사용하지만 더 빠르게 결과를 얻을 수 있습니다.
  //-------------------------------------------------
}


void loop() {
  BLEScanResults foundDevices = pBLEScan->start(SCAN_PERIOD, false);
  //lastSignalTime = millis();  // 타이머 초기화
}

