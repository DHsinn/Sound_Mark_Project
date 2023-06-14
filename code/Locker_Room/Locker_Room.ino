#include "BLEDevice.h"
#include "BLEScan.h"
#include "BLEAdvertisedDevice.h"
#include "BLEBeacon.h"
#include "RTOS.h"
#include "freertos/task.h"

//---------------------------------------------------------------locker room 코드---------------------------------------------------------------

// iBeacon UUID
#define BEACON_UUID "8ec76ea3-6668-48da-9866-75be8bc86f4d"
#define TEST_BEACON_UUID "39ED98FF-2900-441A-802F-9C398FC199D2"

// 스캔 주기 및 스캔 시간
const int SCAN_PERIOD = 10000; // 스캔 주기 (ms, 10초)

//오디오
#include <SoftwareSerial.h>
#include <MP3Player_KT403A.h>
SoftwareSerial mp3(17, 16); //TX, RX  (GIOP 번호)

BLEAdvertising *pAdvertising;   //송출 포인터설정
BLEScan* pBLEScan;   //스캔포인터설정

// 작업 핸들러
TaskHandle_t beaconTaskHandle = NULL;
TaskHandle_t musicTaskHandle = NULL;

// 비콘 검색 작업 함수
void beaconTask(void* parameter) {

  Serial.println("# Task 1 running on core ");
  BLEBeacon oBeacon = BLEBeacon();
  BLEAdvertisementData oAdvertisementData = BLEAdvertisementData();
  pAdvertising = BLEDevice::getAdvertising();

  for (;;) {
    // 비콘 검색 작업
    BLEScanResults foundDevices = pBLEScan->start(SCAN_PERIOD, false);

    if (foundDevices.getCount() > 0) {
      for (int i = 0; i < foundDevices.getCount(); i++) {
        BLEAdvertisedDevice advertisedDevice = foundDevices.getDevice(i);
        // iBeacon 감지 코드
        if (advertisedDevice.haveManufacturerData() && advertisedDevice.getManufacturerData().length() == 25 &&
            advertisedDevice.getManufacturerData()[0] == 0x4c && advertisedDevice.getManufacturerData()[1] == 0x00 &&
            advertisedDevice.getManufacturerData()[2] == 0x02 && advertisedDevice.getManufacturerData()[3] == 0x15 &&
            advertisedDevice.getManufacturerData()[4] == 0x39 && advertisedDevice.getManufacturerData()[5] == 0xed &&
            advertisedDevice.getManufacturerData()[6] == 0x98 && advertisedDevice.getManufacturerData()[7] == 0xff &&
            advertisedDevice.getManufacturerData()[8] == 0x29 && advertisedDevice.getManufacturerData()[9] == 0x00 ) {
          
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
        }
      }
    }
    vTaskDelay(pdMS_TO_TICKS(100)); // 일시 중지 후 100ms 동안 대기
  }
}

// 신호마다 다른 작업 함수
void musicTask(void* parameter) {

  pBLEScan = BLEDevice::getScan();
  Serial.println("# Task 2 running on core ");

  for (;;) {
    BLEScanResults foundDevices = pBLEScan->start(SCAN_PERIOD, false);

    // 비콘 검색 작업 코드
    if (foundDevices.getCount() > 0) {
      for (int i = 0; i < foundDevices.getCount(); i++) {
        BLEAdvertisedDevice advertisedDevice = foundDevices.getDevice(i);

        // iBeacon 감지 코드
        if (advertisedDevice.haveManufacturerData() && advertisedDevice.getManufacturerData().length() == 25 &&
            advertisedDevice.getManufacturerData()[0] == 0x4c && advertisedDevice.getManufacturerData()[1] == 0x00 &&
            advertisedDevice.getManufacturerData()[2] == 0x02 && advertisedDevice.getManufacturerData()[3] == 0x15 &&
            advertisedDevice.getManufacturerData()[4] == 0x39 && advertisedDevice.getManufacturerData()[5] == 0xed &&
            advertisedDevice.getManufacturerData()[6] == 0x98 && advertisedDevice.getManufacturerData()[7] == 0xff &&
            advertisedDevice.getManufacturerData()[8] == 0x29 && advertisedDevice.getManufacturerData()[9] == 0x00 ) {

          // major, minor 값 가져오기
          std::string payload = advertisedDevice.getManufacturerData();
          uint16_t currentMajor = payload[20] << 8 | payload[21];
          uint16_t currentMinor = payload[22] << 8 | payload[23];
          //Serial.printf("Major: %d, Minor: %d\n", currentMajor, currentMinor);

          // 원하는 비콘의 Major, Minor 값에 따라 작업 실행
          if (currentMajor == (1<<8) && currentMinor == 3) {
            // 주변 스캔 신호 처리 작업 실행
            pBLEScan->start(SCAN_PERIOD, true);
          } else if (currentMajor == (3<<8) && currentMinor == 3) {
            // 노래 재생 신호 처리 작업 실행
            SpecifyMusicPlay(1);
          }
        }
      }
    }
    vTaskDelay(pdMS_TO_TICKS(100)); // 일시 중지 후 100ms 동안 대기
  }
}

void setup() {
  Serial.begin(115200);
  BLEDevice::init("RIVO_iBeacon");
  pBLEScan = BLEDevice::getScan();
  BLEDevice::startAdvertising();

  mp3.begin(9600);
  SelectPlayerDevice(0x02);     // Select SD card as the player device. 내 디바이스에 있는 SD카드에서 파일 가져오기
  SetVolume(0x1E);

  // FreeRTOS 작업 생성
  xTaskCreate(beaconTask, "BeaconTask", 4096, NULL, 1, &beaconTaskHandle);
  xTaskCreate(musicTask, "MusicTask", 4096, NULL, 1, &musicTaskHandle);


  // FreeRTOS 스케줄러 시작
  vTaskStartScheduler();

}

void loop() {
}
