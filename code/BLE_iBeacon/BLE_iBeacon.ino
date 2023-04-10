#include "sys/time.h"

#include "BLEDevice.h"
#include "BLEUtils.h"  //ble 관련 해더
#include "BLEBeacon.h" //비콘 헤더
#include "esp_sleep.h" //절전 모드 헤더

// 절전 모드 유지 시간 설정
// 1회 데이터 전송 후 10초 동안 깊은(?) 절전(deep sleep) 모드에 있다가 깨어난다. 
// 즉 10초에 한번씩 깨어나서 비콘 정보를 외부로 송출함
#define GPIO_DEEP_SLEEP_DURATION     10  

// RTC_DATA_ATTR 변수로 저장시 절전 모드에서 깨어나도 값이 유지됨.
// 이전 절전 모드에 진입 때 시간 저장 변수
RTC_DATA_ATTR static time_t last;        
// 재부팅(절전모드 이후 깨어난) 카운트 변수 
// 이 예제에선 카운트 값을 비콘에 실어서 외부로 보냄
RTC_DATA_ATTR static uint32_t bootcount; 

// 외부 송출용 어드버타이징 변수
BLEAdvertising *pAdvertising;

// 현재 시간 저장용 변수
struct timeval now;

// 비콘에 사용할 고유 ID(UUID)
#define BEACON_UUID  "8ec76ea3-6668-48da-9866-75be8bc86f4d"

// 비콘 데이터 설정 함수 핵심!
void setBeacon() {
  // 비콘 클래스 생성
  BLEBeacon oBeacon = BLEBeacon();

  // 제조사 ID 설정
  // 0x4C00은 애플 제조사 ID임
  oBeacon.setManufacturerId(0x4C00);

  // 비콘 고유 ID(UUID) 설정
  oBeacon.setProximityUUID(BLEUUID(BEACON_UUID));

  // 비콘 데이터 bootcount는 리부팅될때마다 1씩 증가하는 값을
  // major minor 값에 추가
  oBeacon.setMajor((bootcount & 0xFFFF0000) >> 16);
  oBeacon.setMinor(bootcount & 0xFFFF);

  // 비콘 데이터를 외부로 송출하기 위해 어드버타이징 변수 생성
  BLEAdvertisementData oAdvertisementData = BLEAdvertisementData();
  BLEAdvertisementData oScanResponseData = BLEAdvertisementData();
  
  // 어드버타이즈 데이터 타입 설정 플레그  
  // 아래서 추가 설명
  oAdvertisementData.setFlags(0x04);
  
  // 외부로 송출할 데이터 변수
  std::string strServiceData = "";  

  // 아래의 값들은 iBeacon 사용시 고정인 듯
  strServiceData += (char)26;     // 데이터 길이가 총 26인데 이중 데이터가 25, 데이터 유형 1바이트
  strServiceData += (char)0xFF;   // 데이터 유형 0xFF

  // 비콘 데이터 가져오기
  // 아래서 추가 설명
  strServiceData += oBeacon.getData();
  
  // 비콘 데이터를 어드버타이징에 설정함.
  oAdvertisementData.addData(strServiceData);   
  pAdvertising->setAdvertisementData(oAdvertisementData);
  pAdvertising->setScanResponseData(oScanResponseData);
}

void setup() {
  //시리얼 모니터 확인용    
  Serial.begin(115200);

  //현재 시간 가져오기
  gettimeofday(&now, NULL);

  //재부팅 카운트 출력
  Serial.printf("start ESP32 %d\n",bootcount++);
  //얼마만에 재부팅(깨어났는지) 출력
  Serial.printf("deep sleep (%lds since last reset, %lds since last boot)\n",now.tv_sec,now.tv_sec-last);

  //현재 시간을 last 변수에 저장
  last = now.tv_sec;
  
  // BLE 초기화
  BLEDevice::init("");

  // 외부로 비콘 송출에 사용되는 어드버타이징 변수를 가져온다.
  pAdvertising = BLEDevice::getAdvertising();
  
  // 비콘 설정 함수 호출!
  setBeacon();

  // 외부로 송출 시작!
  pAdvertising->start();
  
  // 송출을 시작했다고 디버그 메시지 출력
  Serial.println("Hello, let me introduce myself. I'm an employee of rivo Corporation");
  // 100ms, 0.1초 대기
 // delay(100);
  // 송출하던 것을 멈춘다.
  //pAdvertising->stop();

  // 절전 모드에 진입한다고 디버그 메시지 출력
  //Serial.printf("enter deep sleep\n");

  /* 
   절전 모드 진입, 매게변수(파라미터, 옵션)은 마이크로 세컨드(us, 0.000001초)단위 이다.
   위에서 GPIO_DEEP_SLEEP_DURATION에 10을 줬으니 1000000LL * 10 = 10,000,000us = 10초 
   10초후 깨어난다. 깨어나면 바로 이어서 실행되는 것이 아니고 리셋(리부팅)이 되기 때문에
   setup 함수 부터 다시 시작한다.
  */
  //esp_deep_sleep(1000000LL * GPIO_DEEP_SLEEP_DURATION);

  // 절전 모드에 진입 후에는 리부팅 되기 때문에 아래의 메시지는 출력되지 않는다.
  Serial.printf("in deep sleep\n");
}

// loop 사용안함.
void loop() {
}