#include <SoftwareSerial.h>
#include <MP3Player_KT403A.h>

// Note: You must define a SoftwareSerial class object that the name must be mp3, 
//       but you can change the pin number according to the actual situation.
SoftwareSerial mp3(17, 16);    //TX, RX

void setup()
{
    mp3.begin(9600);
    Serial.begin(9600); 
    delay(100);
    
    SelectPlayerDevice(0x02);       // Select SD card as the player device.
    SetVolume(0x0E);                // Set the volume, the range is 0x00 to 0x1E.
}

void loop()
{
    char recvChar = 0;
    while(Serial.available() > 0)
    {
        recvChar = Serial.read();
    }
    Serial.print("Send: ");
    Serial.println( recvChar );
    
    switch (recvChar)
    {
        case '1':
            SpecifyMusicPlay(1);
            Serial.println("Specify the music index to play");
            break;
        case '2':
            PlayPause();
            Serial.println("Pause the MP3 player");
            break;
        case '3':
            PlayResume();
            Serial.println("Resume the MP3 player");
            break;
        case '4':
            PlayNext();
            Serial.println("Play the next song");
            break;
        case '5':
            PlayPrevious();
            Serial.println("Play the previous song");
            break;
        case '6':
            PlayLoop();
            Serial.println("Play loop for all the songs");
            break;
        case '7':
            IncreaseVolume();
            Serial.println("Increase volume");
            break;
        case '8':
            DecreaseVolume();
            Serial.println("Decrease volume");
            break;
        // case '9':
        //     playSongIndex(1);
        //     Serial.println("Call git play function");
        //     break;
        default:
            break;
    }
    
    delay(1000);
    
//    printReturnedData();
}
