#include <ESP8266WiFi.h>
#include <EEPROM.h>

extern "C" {
  #include "user_interface.h"
}

#define btn 0 //GPIO 0 = FLASH BUTTON
  
#define maxCh 13 //max Channel -> US = 11, EU = 13, Japan = 14
#define ledPin 2 //led pin ( 2 = built-in LED)
#define packetRate 5 //min. packets before it gets recognized as an attack

//===== Run-Time variables =====//
unsigned long prevTime = 0;
unsigned long curTime = 0;
unsigned long pkts = 0;
unsigned long deauths = 0;
int curChannel = 1;
bool canBtnPress = true;

void sniffer(uint8_t *buf, uint16_t len) {
  pkts++;
  if(buf[12] == 0xA0 || buf[12] == 0xC0){
    deauths++;
  }
}


//===== SETUP =====//
void setup() {

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
  WiFi.disconnect();
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
    pkts = 0;

    //save changes
    EEPROM.write(2000, curChannel);
    EEPROM.commit();
    
    Serial.println("Ch: " + (String)curChannel + " Pkts: "+(String)pkts);
  }

  //every second
  if(curTime - prevTime >= 1000){
    prevTime = curTime;

    //deauth alarm
    if(deauths > packetRate) digitalWrite(ledPin, LOW);
    else digitalWrite(ledPin, HIGH);

    Serial.println("Ch: " + (String)curChannel + " Pkts: "+(String)pkts);

    //reset counters
    deauths = 0;
    pkts = 0;
  }
  
}
