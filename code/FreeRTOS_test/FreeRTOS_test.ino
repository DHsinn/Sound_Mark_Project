#include "BLEDevice.h"
#include "BLEUtils.h"
#include "BLEScan.h"
#include "BLEAdvertisedDevice.h"
#include "BLEBeacon.h"
#include "esp_sleep.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// iBeacon UUID
#define BEACON_UUID "8ec76ea3-6668-48da-9866-75be8bc86f4d"
#define TEST_BEACON_UUID "39ED98FF-2900-441A-802F-9C398FC199D2"

// 스캔 주기 및 스캔 시간
#define SCAN_INTERVAL 100
#define SCAN_PERIOD 10000

TaskHandle_t scanTaskHandle;
TaskHandle_t beaconTaskHandle;

void scanTask(void *pvParameters);
void beaconTask(void *pvParameters);

void setup() {
  xTaskCreatePinnedToCore(scanTask, "Scan Task", 10000, NULL, 1, &scanTaskHandle, 0); // Scan Task 생성
  xTaskCreatePinnedToCore(beaconTask, "Beacon Task", 10000, NULL, 2, &beaconTaskHandle, 1); // Beacon Task 생성
}

void loop() {
  // 아무 작업도 수행하지 않음
  vTaskDelay(1000 / portTICK_PERIOD_MS); // 1초 대기
}

void scanTask(void *pvParameters) {
  BLEScan* pBLEScan = BLEDevice::getScan(); // 스캔 객체 생성
  pBLEScan->setInterval(SCAN_INTERVAL);
  pBLEScan->setWindow(SCAN_INTERVAL - 1);
  pBLEScan->setActiveScan(true); // 활성화된 스캔

  while (1) {
    BLEScanResults foundDevices = pBLEScan->start(SCAN_PERIOD, false); // 스캔 시작
    // 스캔 결과 처리

    for (int i = 0; i < foundDevices.getCount(); i++) {
      BLEAdvertisedDevice device = foundDevices.getDevice(i);
    }

    vTaskDelay(1000 / portTICK_PERIOD_MS); // 1초 대기
  }
}

void beaconTask(void *pvParameters) {
  while (1) {
    // 비콘 작업 수행
    vTaskDelay(1000 / portTICK_PERIOD_MS); // 1초 대기
  }
}
