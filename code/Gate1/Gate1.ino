#include "BLEDevice.h"
#include "BLEUtils.h"
#include "BLEScan.h"
#include "BLEAdvertisedDevice.h"
#include "BLEBeacon.h"
#include "esp_sleep.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


//-------------------------------------------------------------------------------게이트1------------------------------------------------------------------

// UUID
#define BEACON_UUID "8ec76ea3-6668-48da-9866-75be8bc86f4d"
#define TEST_BEACON_UUID "39ED98FF-2900-441A-802F-9C398FC199D2"

// 스캔 주기 및 스캔 시간
#define SCAN_WINDOW 299   //스캔 창크기
#define SCAN_INTERVAL 300   //리턴 주기

// command
#define SCAN 1<<8
#define INFO 2<<8
#define LOCATE 3<<8

//오디오
#include "SoftwareSerial.h"
#include "MP3Player_KT403A.h"
SoftwareSerial mp3(17, 16); //TX, RX  (GIOP 번호)

BLEScan* pBLEScan;
BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
BLEBeacon oBeacon = BLEBeacon();
BLEAdvertisementData oAdvertisementData = BLEAdvertisementData();

bool beacon_detected = false;

SemaphoreHandle_t semaphore;
TaskHandle_t scanTaskHandle;
TaskHandle_t beaconTaskHandle;


class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  public:
  bool isPlaying = false;
  unsigned long startTime = 0;
  
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    std::string manufacturerData = advertisedDevice.getManufacturerData();
    //Serial.printf("Advertised Device: %s \n", advertisedDevice.toString().c_str());

    //Serial.printf("%d\n", advertisedDevice.getcount());
    if (advertisedDevice.haveManufacturerData() && manufacturerData.length() == 25 &&
        manufacturerData[0] == 0x4c && manufacturerData[1] == 0x00 &&
        manufacturerData[2] == 0x02 && manufacturerData[3] == 0x15 &&
        manufacturerData[4] == 0x39 && manufacturerData[5] == 0xed &&
        manufacturerData[6] == 0x98 && manufacturerData[7] == 0xff &&
        manufacturerData[8] == 0x29 && manufacturerData[9] == 0x00) {
        
        //Serial.println("신호받음");
        beacon_detected = true;

      //신호값 받기
        uint16_t major = manufacturerData[20] << 8 | manufacturerData[21];
        uint16_t minor = manufacturerData[22] << 8 | manufacturerData[23];

        Serial.printf("메이저 : %d\n", major);
        //Serial.printf("마이너 : %d\n", minor);
        
        if(major == (LOCATE)+1 && (minor == (4<<8)+1 || minor == (4<<8) || minor == 0) && !isPlaying){
          Serial.println("들어오지않음?");
          SpecifyMusicPlay(1);
          isPlaying = true;
          startTime = millis();
        }
        else if(major == (LOCATE)+2 && (minor == (4<<8)+1 || minor == (4<<8) || minor == 0) && !isPlaying){
          SpecifyMusicPlay(2);
          isPlaying = true;
          startTime = millis();
        }
        else if(major == (LOCATE)+3 && (minor == (4<<8)+1 || minor == (4<<8) || minor == 0) && !isPlaying){
          SpecifyMusicPlay(3);
          isPlaying = true;
          startTime = millis();
        }
      }
      else{
        beacon_detected = false;
      }
      
    // SCAN_INTERVAL동안 노래재생 못하게 하기
    if (isPlaying && (millis() - startTime >= SCAN_INTERVAL)) {
      isPlaying = false;
    }
  }
};

//MyAdvertisedDeviceCallbacks callbacks;

void scanTask(void* pvParameters){
  for (;;) {
      //if (xSemaphoreTake(semaphore, portMAX_DELAY) == pdTRUE) {
        pBLEScan->start(true);
      //xSemaphoreGive(semaphore);
    }
    vTaskDelay(10 / portTICK_PERIOD_MS);  // Delay for 0.01 second
  }

void beaconTask(void* pvParameters){
  for (;;) {
    //if (xSemaphoreTake(semaphore, portMAX_DELAY) == pdTRUE){
      if(beacon_detected){
      oBeacon.setManufacturerId(0x4c00);
      oBeacon.setProximityUUID(BLEUUID(BEACON_UUID));
      oBeacon.setMajor(INFO);  //0000 0001 0000 0000
      oBeacon.setMinor((4<<8)+1); //0000 0004 0000 0001 0x0401

      oAdvertisementData.setFlags(0x04);
      oAdvertisementData.setManufacturerData(oBeacon.getData());

      pAdvertising->setAdvertisementData(oAdvertisementData);
      pAdvertising->setScanResponseData(oAdvertisementData);
      pAdvertising->start();

     // xSemaphoreGive(semaphore);
    }
    //else{
    //  pAdvertising->stop();
    //}
    vTaskDelay(10 / portTICK_PERIOD_MS);  // Delay for 0.01 second
    //vTaskDelete(NULL);
  }
}

void setup() {
  BLEDevice::init("RIVO_iBeacon");
  Serial.begin(115200);

  semaphore = xSemaphoreCreateMutex();

  //---------------------mp3 구성-------------------
  mp3.begin(9600);
  delay(100);
  
  SelectPlayerDevice(0x02);       // Select SD card as the player device. 내 디바이스에 있는 SD카드에서 파일 가져오기
  SetVolume(0x1E);        //볼륨설정하기 최대 0x1E
  //-------------------------------------------------

  pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(SCAN_INTERVAL);
  pBLEScan->setWindow(SCAN_WINDOW);
  pBLEScan->setActiveScan(true);

  // Create a task for scanning
  xTaskCreatePinnedToCore(scanTask, "Scan Task", 2048, NULL, 1, &scanTaskHandle, 0); // Scan Task 생성
  xTaskCreatePinnedToCore(beaconTask, "Beacon Task", 2048, NULL, 1, &beaconTaskHandle, 1); // Beacon Task 생성

  //vTaskStartScheduler();
}

void loop() {
  //vTaskDelay(100 / portTICK_PERIOD_MS); // 0.1초 대기
}
