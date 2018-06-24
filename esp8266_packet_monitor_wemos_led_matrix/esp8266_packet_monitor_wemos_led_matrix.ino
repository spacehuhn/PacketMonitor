#include <ESP8266WiFi.h>
#include <Wire.h>
#include <EEPROM.h>
#include "WEMOS_Matrix_LED.h"

const uint64_t IMAGES[] PROGMEM = {
    0x000001ff41000000,
    0x0000f191919f0000,
    0x0000ff9191910000,
    0x0000ff1010f00000,
    0x00009f9191f10000,
    0x00009f9191ff0000,
    0x0000ff8080800000,
    0x0000ff9191ff0000,
    0x0000ff9191f10000,
    0xff8181ff0001ff41,
    0x01ff41000001ff41,
    0xf191919f0001ff41,
    0xff9191910001ff41,
    0xff1010f00001ff41
};
const int IMAGES_LEN = sizeof(IMAGES)/8;

MLED mled(1); //set intensity max 7

extern "C" {
    #include "user_interface.h"
}

#define btn 0 //GPIO 0 = FLASH BUTTON

#define maxCh 14 //max Channel -> US = 11, EU = 13, Japan = 14
#define packetRate 5 //min. packets before it gets recognized as an attack

unsigned long prevTime = 0;
unsigned long curTime = 0;
unsigned long pkts = 0;
unsigned long deauths = 0;
int curChannel = 1;
unsigned long maxVal = 0;
double multiplicator = 0.0;
bool canBtnPress = true;

int val[8];

void displayImage(uint64_t image)
{
    for (int i = 0; i < 8; i++)
    {
        byte row = (image >> i * 8) & 0xFF;
        for (int j = 0; j < 8; j++)
        {
            mled.dot(i, j, bitRead(row, j));
            mled.display();
        }
    }
}
void display_Channel(int channel)
{
    int i = channel-1;

    uint64_t image;
    memcpy_P(&image, &IMAGES[i], 8);

    displayImage(image);
}

void sniffer(uint8_t *buf, uint16_t len)
{
    pkts++;
    if(buf[12] == 0xA0 || buf[12] == 0xC0)
    {
        deauths++;
    }
}

void getMultiplicator()
{
    maxVal = 1;
    for(int i=0;i<8;i++)
    {
        if(val[i] > maxVal)
        {
            maxVal = val[i];
        }
    }
    if(maxVal > 8)
    {
        multiplicator = (double)8/(double)maxVal;
    }
    else
    {
        multiplicator = 1;
    }
}


void setup() {
  Serial.begin(115200);

    /* load last saved channel */
    EEPROM.begin(4096);
    curChannel = EEPROM.read(2000);
    if(curChannel < 1 || curChannel > maxCh)
    {
        curChannel = 1;
        EEPROM.write(2000, curChannel);
        EEPROM.commit();
    }

    /* set pin modes for button and LED */
    pinMode(btn, INPUT);
    pinMode(BUILTIN_LED, OUTPUT);

    /* setup wifi */
    wifi_set_opmode(STATION_MODE);
    wifi_promiscuous_enable(0);
    WiFi.disconnect();
    wifi_set_promiscuous_rx_cb(sniffer);
    wifi_set_channel(curChannel);
    wifi_promiscuous_enable(1);

    display_Channel(curChannel);
    delay(1000);
    mled.clear();

    Serial.println("starting!");
}

void loop() {
    curTime = millis();

    //on button release
    if(digitalRead(btn) == LOW)
    {
        if(canBtnPress)
        {
            canBtnPress = false;
        }
    }
    else if(!canBtnPress)
    {
        canBtnPress = true;
  
        //switch channel
        curChannel++;
        if(curChannel > maxCh)
        {
            curChannel = 1;
        }
        wifi_set_channel(curChannel);
        for(int i=0;i<8;i++)
        {
            val[i] = 0;
        }
        pkts = 0;
        multiplicator = 1;
  
        //save changes
        EEPROM.write(2000, curChannel);
        EEPROM.commit();
  
        display_Channel(curChannel);
        delay(1000);
  
        for(int y=0;y<8;y++)
        {
            for(int x=0;x<8;x++)
            {
                mled.dot(x,y); // draw dot
                mled.display();
                delay(10);
                mled.dot(x,y,0);//clear dot
                mled.display();
                delay(10);
            }
        }

        //draw display
        mled.clear();
        for(int i=0;i<8;i++) 
        {
            if ((val[i]*multiplicator) > 0)
            {
                mled.drawline(i, 0, i, ((val[i]*multiplicator))-1);
            }
        }
            mled.display();
            Serial.println("Ch: " + (String)curChannel + " Pkts: "+(String)pkts);
    }

    //every second
    if(curTime - prevTime >= 1000)
    {
        prevTime = curTime;

        //move every packet bar one pixel to the left
        for(int i=0;i<7;i++)
        {
            val[i] = val[i+1];
        }
        val[7] = pkts;

        //recalculate scaling factor
        getMultiplicator();

        //deauth alarm
        if(deauths > packetRate)
        {
            digitalWrite(BUILTIN_LED, LOW);
        }
        else
        {
            digitalWrite(BUILTIN_LED, HIGH);
        }

        //draw display
        mled.clear();
        for(int i=0;i<8;i++)
        {
            if ((val[i]*multiplicator) > 0)
            {
                mled.drawline2(i, 0, i, ((val[i]*multiplicator))-1);
            }
        }
        mled.display();
        Serial.println("Ch: " + (String)curChannel + " Pkts: "+(String)pkts);
        
        //reset counters
        deauths = 0;
        pkts = 0;
    }
}
