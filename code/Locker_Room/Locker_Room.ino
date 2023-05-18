#include <Arduino.h>

#include "BLEDevice.h"
#include "BLEUtils.h"
#include "BLEScan.h"
#include "BLEAdvertisedDevice.h"
#include "BLEBeacon.h"

#include <SoftwareSerial.h>
#include <MP3Player_KT403A.h>

#define SCAN_INTERVAL 100

#define BEACON_UUID "8ec76ea3-6668-48da-9866-75be8bc86f4d"
#define TEST_BEACON_UUID "39ED98FF-2900-441A-802F-9C398FC199D2"

int previousMajor = 40; // 이전에 체크한 비콘의 Major 값
int previousMinor = 15; // 이전에 체크한 비콘의 Minor 값
const int SCAN_PERIOD = 10000; // 스캔 주기 (ms)
int count = 0;  //loop 횟수


SoftwareSerial mp3(17, 16); //TX, RX  (GIOP 번호)

BLEAdvertising *pAdvertising;   //송출 포인터설정
BLEScan* pBLEScan;   //스캔포인터설정
bool isBeaconDetected = false;   //아이비콘을 찾았다면 true 값으로 변경
bool Scan = false;
bool Playsong = false;


unsigned long t1Interval = 1000; // Task 1 간격 (밀리초)
unsigned long t2Interval = 2000; // Task 2 간격 (밀리초)

unsigned long t1PreviousMillis = 0; // Task 1 이전 실행 시간 (밀리초)
unsigned long t2PreviousMillis = 0; // Task 2 이전 실행 시간 (밀리초)

void t1Task() {
  unsigned long currentMillis = millis();
  if (currentMillis - t1PreviousMillis >= t1Interval) {
    // Task 1 로직 실행
    t1PreviousMillis = currentMillis;
  }
}

void t2Task() {
  unsigned long currentMillis = millis();
  if (currentMillis - t2PreviousMillis >= t2Interval) {
    // Task 2 로직 실행
    t2PreviousMillis = currentMillis;
  }
}

void setup() {
  // 시리얼 통신 초기화 등 초기 설정 작업
  Serial.begin(115200);


  BLEDevice::init("RIVO_iBeacon"); // BLE을 생성
  pBLEScan = BLEDevice::getScan(); // 새로운 스캔 객체 생성
  //pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(SCAN_INTERVAL);
  pBLEScan->setWindow(SCAN_INTERVAL - 1);
  pBLEScan->setActiveScan(true);
}

void loop() {
  // Task 1 실행
  t1Task();

  // Task 2 실행
  t2Task();

  // 추가적인 로직 및 작업

}
