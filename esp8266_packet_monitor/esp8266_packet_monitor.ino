#include <ESP8266WiFi.h>
#include <Wire.h>
#include <EEPROM.h>

#include "SSD1306.h"
#include "SH1106.h"

extern "C" {
  #include "user_interface.h"
}

//===== SETTINGS =====//
//create display(Adr, SDA-pin, SCL-pin)
SSD1306 display(0x3c, 5, 4); //GPIO 5 = D1, GPIO 4 = D2
//SH1106 display(0x3c, 5, 4);

#define btn 0 //GPIO 0 = FLASH BUTTON
  
#define maxCh 13 //max Channel -> US = 11, EU = 13, Japan = 14
#define ledPin 2 //led pin ( 2 = built-in LED)
#define packetRate 5 //min. packets before it gets recognized as an attack

#define flipDisplay true


//===== Run-Time variables =====//
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

void getMultiplicator(){
  maxVal = 1;
  for(int i=0;i<128;i++){
    if(val[i] > maxVal) maxVal = val[i];
  }
  if(maxVal > 53) multiplicator = (double)53/(double)maxVal;
  else multiplicator = 1;
}

//===== SETUP =====//
void setup() {
  /* start Display */
  display.init();
  if(flipDisplay) display.flipScreenVertically();

  /* show start screen */
  display.clear();
  display.setFont(ArialMT_Plain_16);
  display.drawString(0, 0, "Packet-");
  display.drawString(0, 16, "Monitor");
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 40, "Copyright (c) 2017");
  display.drawString(0, 50, "Stefan Kremser");
  display.display();
  delay(2500);

  /* start Serial */
  Serial.begin(115200);

  /* load last saved channel */
  EEPROM.begin(4096);
  curChannel = EEPROM.read(2000);
  if(curChannel < 1 || curChannel > maxCh){
    curChannel = 1;
    EEPROM.write(2000, curChannel);
    EEPROM.commit();
  }

  /* set pin modes for button and LED */
  pinMode(btn, INPUT);
  pinMode(ledPin, OUTPUT);

  /* setup wifi */
  wifi_set_opmode(STATION_MODE);
  wifi_promiscuous_enable(0);
  wifi_set_promiscuous_rx_cb(sniffer);
  wifi_set_channel(curChannel);
  wifi_promiscuous_enable(1);
  
  Serial.println("starting!");
}

//===== LOOP =====//
void loop() {
  curTime = millis();

  //on button release
  if(digitalRead(btn) == LOW){
    if(canBtnPress) canBtnPress = false;
  }else if(!canBtnPress){
    canBtnPress = true;

    //switch channel
    curChannel++;
    if(curChannel > maxCh) curChannel = 1;
    wifi_set_channel(curChannel);
    for(int i=0;i<128;i++) val[i] = 0;
    pkts = 0;
    multiplicator = 1;

    //save changes
    EEPROM.write(2000, curChannel);
    EEPROM.commit();

    //draw display
    display.clear();
    for(int i=0;i<128;i++) display.drawLine(i, 64, i, 64-val[i]*multiplicator);
    display.drawString(0, 0, " Ch: " + (String)curChannel + " Pkts: "+(String)pkts);
    display.display();
  }

  //every second
  if(curTime - prevTime >= 1000){
    prevTime = curTime;

    //move every packet bar one pixel to the left
    for(int i=0;i<127;i++){
      val[i] = val[i+1];
    }
    val[127] = pkts;

    //recalculate scaling factor
    getMultiplicator();

    //deauth alarm
    if(deauths > packetRate) digitalWrite(ledPin, LOW);
    else digitalWrite(ledPin, HIGH);

    //draw display
    display.clear();
    for(int i=0;i<128;i++) display.drawLine(i, 64, i, 64-val[i]*multiplicator);
    display.drawString(0, 0, " Ch: " + (String)curChannel + " Pkts: "+(String)pkts);
    display.display();

    //reset counters
    deauths = 0;
    pkts = 0;
  }
  
}
