#include <ESP8266WiFi.h>
#include <Wire.h>
#include <EEPROM.h>

extern "C" {
  #include "user_interface.h"
}

//include the library you need
#include "SSD1306.h"
//#include "SH1106.h"

#define btn 0

//create display(Adr, SDA-pin, SCL-pin)
SSD1306 display(0x3c, D1, D2);
//SH1106 display(0x3c, D2, D1);
  
//===== SETTINGS =====//
#define maxCh 13 //max Channel -> US = 11, EU = 13, Japan = 14
#define ledPin 2 //led pin ( 2 = built-in LED)
#define packetRate 5 //min. packets before it gets recognized as an attack

#define flipDisplay true

unsigned long prevTime = 0;
unsigned long curTime = 0;
unsigned long pkts = 0;
unsigned long deauths = 0;
int curChannel = 1;
unsigned long maxVal = 0;
double multiplicator = 0.0;
bool canBtnPress = true;

int val[128];

void sniffer(uint8_t *buf, uint16_t len) {
  pkts++;
  if(buf[12] == 0xA0 || buf[12] == 0xC0){
    deauths++;
  }
}

void setup() {
  Serial.begin(115200);

  EEPROM.begin(4096);
  curChannel = EEPROM.read(2000);
  if(curChannel < 1 || curChannel > maxCh){
    curChannel = 1;
    EEPROM.write(2000, curChannel);
    EEPROM.commit();
  }
  
  display.init();
  if(flipDisplay) display.flipScreenVertically();
  pinMode(btn, INPUT);

  pinMode(ledPin, OUTPUT);

  wifi_set_opmode(STATION_MODE);
  wifi_promiscuous_enable(0);
  WiFi.disconnect();
  wifi_set_promiscuous_rx_cb(sniffer);
  wifi_set_channel(curChannel);
  wifi_promiscuous_enable(1);
  
  Serial.println("starting!");
}

void getMultiplicator(){
  maxVal = 1;
  for(int i=0;i<128;i++){
    if(val[i] > maxVal) maxVal = val[i];
  }
  if(maxVal > 54) multiplicator = (double)54/(double)maxVal;
  else multiplicator = 1;
}

void loop() {
  curTime = millis();
  
  if(digitalRead(btn) == LOW){
    if(canBtnPress){
      curChannel++;
      if(curChannel > maxCh) curChannel = 1;
      wifi_set_channel(curChannel);
      for(int i=0;i<128;i++) val[i] = 0;
      pkts = 0;
      multiplicator = 1;
      canBtnPress = false;

      EEPROM.write(2000, curChannel);
      EEPROM.commit();

      display.clear();
      for(int i=0;i<128;i++) display.drawLine(i, 64, i, 64-val[i]*multiplicator);
      display.drawString(0, 0, " Ch: " + (String)curChannel + " Pkts: "+(String)pkts);
      display.display();
    }
  }else if(!canBtnPress){
    canBtnPress = true;
  }

  if(curTime - prevTime >= 1000){
    prevTime = curTime;

    for(int i=0;i<127;i++){
      val[i] = val[i+1];
    }
    val[127] = pkts;

    getMultiplicator();

    if(deauths > packetRate) digitalWrite(ledPin, LOW);
    else digitalWrite(ledPin, HIGH);

    display.clear();
    for(int i=0;i<128;i++) display.drawLine(i, 64, i, 64-val[i]*multiplicator);
    display.drawString(0, 0, " Ch: " + (String)curChannel + " Pkts: "+(String)pkts);
    display.display();
    
    deauths = 0;
    pkts = 0;
  }
  
}
