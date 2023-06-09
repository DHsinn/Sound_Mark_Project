// 스피커
#include "pitches.h" //음계

#define speakerpin 23   //스피커 연결 핀번호 (GIOP 번호)


int melody[] = {NOTE_G7,NOTE_G7,NOTE_A7,NOTE_A7,NOTE_G7,NOTE_G7,NOTE_E7,NOTE_G7,
NOTE_G7,NOTE_E7,NOTE_E7,NOTE_D7,NOTE_G7,NOTE_G7,NOTE_A7,NOTE_A7,
NOTE_G7,NOTE_G7,NOTE_E7,NOTE_G7,NOTE_E7,NOTE_D7,NOTE_E7,NOTE_C7};

int nds[] = {4,4,4,4,4,4,2,4,4,4,4,1,4,4,4,4,4,4,2,4,4,4,4};


//오디오 모듈
#include <SoftwareSerial.h>
#include <MP3Player_KT403A.h>
SoftwareSerial mp3(17, 16); //TX, RX  (GIOP 번호)


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
#define SERVICE_UUID          "8fc64f89-8973-45ef-8469-3d52b3cea272" //서비스 UUID     BLE 따라서 추가해본거

//비콘 데이터 설정 함수의 핵심
void setBeacon() {

  BLEBeacon oBeacon = BLEBeacon();  //비콘 클래스 생성
  oBeacon.setManufacturerId(0x4C00); // fake Apple 0x004C LSB (ENDIAN_CHANGE_U16!)      // 제조사 ID 설정회사 코드 작성하는것
  oBeacon.setProximityUUID(BLEUUID(BEACON_UUID));        //위 define에 정의한 아이디로 UUID 변경
  oBeacon.setMajor(10);                                  //Major 값
  oBeacon.setMinor(5);                                   //minor 값
  //oBeacon.setMajor((bootcount & 0xFFFF0000) >> 16);      //재부팅 될때마다 1씩 증가하는 값을 Major 값에 추가
  //oBeacon.setMinor(bootcount&0xFFFF);                    //재부팅 될때마다 1씩 증가하는 값을 Minor 값에 추가
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

  Serial.begin(9600);       //시리얼 통신을 위해 속도 맞춰주기 시리얼 모니터 확인용

  gettimeofday(&now, NULL);    // 현재 시간 가져오기

  //Serial.printf("start ESP32 %d\n",bootcount++);  //재부팅 카운트 출력
  Serial.printf("start ESP32 \n");  //시작 출력

  //Serial.printf("deep sleep (%lds since last reset, %lds since last boot)\n",now.tv_sec,now.tv_sec - last);  // 얼마만에 재부팅 했는지 출력

  last = now.tv_sec;  // 현재 시간을 last에 저장
  
  // Create the BLE Device
  BLEDevice::init("RIVO 비콘");    //BLE 초기화,생성 (이름설정가능)

  // Create the BLE Server
  // BLEServer *pServer = BLEDevice::createServer(); // <-- no longer required to instantiate BLEServer, less flash and ram usage

  //pAdvertising = BLEDevice::getAdvertising();    //외부로 비콘 송출에 사용되는 어드버타이징 변수를 가져온다.
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();      //바로 윛주석 BLE 버전으로 추가
  BLEDevice::startAdvertising();  //이거 써줘야 BLEDevice 값이 들어감
  
  setBeacon();   //비콘 설정 함수 호출
  //Start advertising
  pAdvertising->start();   // 외부로 송출 시작
  Serial.println("Advertizing started.!");   // 송출 시작했다고 메시지 출력
  //delay(100);  // 0.1초 대기
  //pAdvertising->stop();  //송출 정지
  //Serial.printf("enter deep sleep\n");  // 절전 모드 진입한다고 메시지 출력

  //esp_deep_sleep(1000000LL * GPIO_DEEP_SLEEP_DURATION);   //절전 모드 진입, 10초후 깨어남 깨어나서 (리셋 이기 때문에) 다시 setup 함수부터 시작
  //Serial.printf("in deep sleep\n");
  
  /*
  for(int tn = 0; tn<24; tn++){
    int nd=1000/nds[tn];
    tone(speakerpin, melody[tn],nd);
    int pbn = nd*1.30;
    delay(pbn);
    noTone(speakerpin);
  }  //학교종이 땡땡땡 출력
  */

  mp3.begin(9600);
    Serial.begin(9600); 
    delay(100);
    
    SelectPlayerDevice(0x02);       // Select SD card as the player device.
    SetVolume(0x0E);                // Set the volume, the range is 0x00 to 0x1E.
}


void loop() {

/* //스피커 부저 테스트
  if(Serial.available()){
    byte b = Serial.read();

    if(b == '0'){
      tone(speakerpin,500,500);
      delay(2000);
    }
    if(b == '1'){
      tone(speakerpin,0,0);
    }
  }
*/

/*// mp3 파일 재생 명령어
  char recvChar = 0;
    while(Serial.available() > 0)
    {
        recvChar = Serial.read();
    }
    Serial.print("Send: ");
    Serial.println( recvChar );
    
    switch (recvChar)
    {
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
        case 'a':
            Serial.println("화장실 입니다.");
            break;
        default:
            break;
    }
    
    delay(1000);
*/
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