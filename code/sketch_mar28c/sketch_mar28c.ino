/*
   Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleScan.cpp
   Ported to Arduino ESP32 by pcbreflux
*/

 
/*
   Create a BLE server that will send periodic iBeacon frames.
   The design of creating the BLE server is:
   1. Create a BLE Server
   2. Create advertising data
   3. Start advertising.
   4. wait
   5. Stop advertising.
   6. deep sleep
   
*/
#include "sys/time.h"

#include "BLEDevice.h"
#include "BLEUtils.h"  //ble 관련 헤더
#include "BLEBeacon.h"  //비콘 헤더
#include "esp_sleep.h"  //절전 모드 헤더

#define GPIO_DEEP_SLEEP_DURATION     10  // 절전 모드 유지 시간 설정 1회 데이터 전송 후 10초 동안 절전 후 깨어남 즉 10초에 한번씩 비콘 정보 송출
RTC_DATA_ATTR static time_t last;        //RTC_DATA_ATTR를 변수로 저장시 절전 모드에서 깨어나도 값이 유지됨, 이전 절전 모드에 진입 때 시간 저장 변수
RTC_DATA_ATTR static uint32_t bootcount; // 재부팅 카운트 변수, 요기 예제에선 카운트 값을 비콘에 실어서 외부로 보냄

BLEAdvertising *pAdvertising;   // 외부 송출용 어드버타이징 변수
struct timeval now;  //현재 시간 저장용 변수

//비콘에 사용할 고유 ID
#define BEACON_UUID           "c2188fd0-cd3a-11ed-afa1-0242ac120002" // UUID 1 128-Bit (may use linux tool uuidgen or random numbers via https://www.uuidgenerator.net/)

//비콘 데이터 설정 함수의 핵심
void setBeacon() {

  BLEBeacon oBeacon = BLEBeacon();  //비콘 클래스 생성
  oBeacon.setManufacturerId(0x4C00); // fake Apple 0x004C LSB (ENDIAN_CHANGE_U16!)      // 제조사 ID 설정회사 코드 작성하는것
  oBeacon.setProximityUUID(BLEUUID(BEACON_UUID));        //위 define에 정의한 아이디로 UUID 변경
  oBeacon.setMajor((bootcount & 0xFFFF0000) >> 16);      //재부팅 될때마다 1씩 증가하는 값을 Major 값에 추가
  oBeacon.setMinor(bootcount&0xFFFF);                    //재부팅 될때마다 1씩 증가하는 값을 Minor 값에 추가
  BLEAdvertisementData oAdvertisementData = BLEAdvertisementData();     //비콘 데이터 외부 송출 위한 어드버타이징 변수 생성
  BLEAdvertisementData oScanResponseData = BLEAdvertisementData();      // 스캔 요청이 오면 반응 할 스캔응답 변수 생성
  
  oAdvertisementData.setFlags(0x04); // BR_EDR_NOT_SUPPORTED 0x04       //어드버타이즈 데이터 타입 설정 플래그 (BR/EDR 미지원)
  
  // 비콘 데이터들 어드버타이징 변수에 추가하는 부분
  std::string strServiceData = "";  // 외부로 송출할 데이터 변수
  
  //아래 값들은 iBeacon 사용시 고정인 듯?
  strServiceData += (char)26;     // Len 데이터 길이가 총 26인데 이중 데이터가 25, 데이터 유형 1바이트
  strServiceData += (char)0xFF;   // Type 데이터 유형 0xFF
  strServiceData += oBeacon.getData();  // 비콘 데이터 가져오기
  oAdvertisementData.addData(strServiceData);  // 비콘 데이터를 어드버타이징에 설정
  
  pAdvertising->setAdvertisementData(oAdvertisementData);
  pAdvertising->setScanResponseData(oScanResponseData);
}

void setup() {

    
  Serial.begin(115200);       //시리얼 통신을 위해 속도 맞춰주기 시리얼 모니터 확인용
  gettimeofday(&now, NULL);    // 현재 시간 가져오기

  Serial.printf("start ESP32 %d\n",bootcount++);  //재부팅 카운트 출력

  Serial.printf("deep sleep (%lds since last reset, %lds since last boot)\n",now.tv_sec,now.tv_sec-last);  // 얼마만에 재부팅 했는지 출력

  last = now.tv_sec;  // 현재 시간을 last에 저장
  
  // Create the BLE Device
  BLEDevice::init("");    //BLE 초기화

  // Create the BLE Server
  // BLEServer *pServer = BLEDevice::createServer(); // <-- no longer required to instantiate BLEServer, less flash and ram usage

  pAdvertising = BLEDevice::getAdvertising();    //외부로 비콘 송출에 사용되는 어드버타이징 변수를 가져온다.
  
  setBeacon();   //비콘 설정 함수 호출
   // Start advertising
  pAdvertising->start();   // 외부로 송출 시작
  Serial.println("Advertizing started...");   // 송출 시작했다고 메시지 출력
  //delay(100);  // 0.1초 대기
  //pAdvertising->stop();  //송출 정지
  //Serial.printf("enter deep sleep\n");  // 절전 모드 진입한다고 메시지 출력

  //esp_deep_sleep(1000000LL * GPIO_DEEP_SLEEP_DURATION);   //절전 모드 진입, 10초후 깨어남 깨어나서 (리셋 이기 때문에) 다시 setup 함수부터 시작
  //Serial.printf("in deep sleep\n");
  
}

void loop() {
  /*
  -사용자 입장-
  1. 스마트폰 앱으로 사운드 마크들 스캔 정보 확인 (장애물, 화장실, 출입문, 책상 등) 특정 사운드마크에 알림 요청 장애물은 사용자가 다가오면 자동으로 위험 신호 발생
  
  -하드웨어 입장-
  1. 정보조회 요청 들어오면 자신의 정보 전송 (장애물, 화장실, 출입문, 책상 등), 상세정보 요청이 들어오면 상세정보 전송
  2. 위치확인 요청을 받으면 알림 사운드 재생
  
  -하드웨어 입장(관리자)-
  1. 요청에 맞게 응답 (기기상태, 배터리 레벨, 레이블 등)

  -사용자 앱-
  1. 사운드마크 스캔 및 대략적인 거리 계산 (일정거리 이내 사운드 마크만 알리는 옵션 제공)
  2. 클라우드 서버 통해 사운드마크 상세 정보 조회가능, 접속 불가시 사운드마크에 자신의 언어 설정에 맞는 상세 정보를 요청하고 수신
  3. 본인의 선호 언어 지정 (없다면 기본 영어 정보를 받아서 번역)
  4. 리보 앱 모드로 단축 기능 지원

  -관리자 앱-
  1. 설정 : 사운드마크별 레이블 설정, 권한 설정, 추가 정보를 클라우드 서버에 저장
  2. 관리 : 기기 상태, 배터리 레벨, 기기에 저장된 레이블, 서버에 저장된 추가 정보 등을 조회
  3. 위치확인 알림 사운드 요청

  -서버-
  1. 관리자 앱에서 사운드마크 설정시 사운드마크별 관련 정보를 서버에 저장
  2. 사용자 앱에서 조회시 해당 사운드마크 정보 전달 (사용자 언어설정에 맞게 해당 데이터를 전달)

  -기타-
  1. 비컨 이름길이 최대 6+글자
  2. 배터리 레벨 확인
  3. 작동거리 30m
  4. 설정 사운드 종류 10+개
  5. 배터리 사용기간 1달~1년
  */
  
}