#if defined(ESP8266)
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#endif
#include <Wire.h>
#include <EEPROM.h>

#include "SSD1306.h"
#include "SH1106.h"

#if defined(ESP8266)
extern "C" {
  #include "user_interface.h"
}
#else
#include "esp_wifi.h"
const wifi_promiscuous_filter_t filt={
    .filter_mask=WIFI_PROMIS_FILTER_MASK_MGMT|WIFI_PROMIS_FILTER_MASK_DATA
};
  
typedef struct {
  uint8_t mac[6];
} __attribute__((packed)) MacAddr;

typedef struct {
  int16_t fctl;
  int16_t duration;
  MacAddr da;
  MacAddr sa;
  MacAddr bssid;
  int16_t seqctl;
  unsigned char payload[];
} __attribute__((packed)) WifiMgmtHdr;
#endif

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

#if defined(ESP8266)
void sniffer(uint8_t *buf, uint16_t len) {
#else
void sniffer(void* buf, wifi_promiscuous_pkt_type_t type) {
#endif
  pkts++;
#if defined(ESP8266)
  if(buf[12] == 0xA0 || buf[12] == 0xC0){
    deauths++;
  }
#else
  if (type == WIFI_PKT_MGMT) {
    wifi_promiscuous_pkt_t *p = (wifi_promiscuous_pkt_t*)buf;
    int len = p->rx_ctrl.sig_len;
    WifiMgmtHdr *wh = (WifiMgmtHdr*)p->payload;
    len -= sizeof(WifiMgmtHdr);
    if (len < 0) return;
    int fctl = ntohs(wh->fctl);
    if (fctl & 0x0F00 == 0x0A00 || fctl & 0x0F00 == 0x0C00) {
      deauths++;
      Serial.println("DEAUTH:");
    }
  }
#endif
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
#if defined(ESP8266)
  wifi_set_opmode(STATION_MODE);
  wifi_promiscuous_enable(0);
  WiFi.disconnect();
  wifi_set_promiscuous_rx_cb(sniffer);
  wifi_set_channel(curChannel);
  wifi_promiscuous_enable(1);
#else
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  esp_wifi_init(&cfg);
  esp_wifi_set_country(WIFI_COUNTRY_EU);
  esp_wifi_set_storage(WIFI_STORAGE_RAM);
  esp_wifi_set_mode(WIFI_MODE_NULL);
  esp_wifi_start();
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_promiscuous_filter(&filt);
  esp_wifi_set_promiscuous_rx_cb(&sniffer);
  esp_wifi_set_channel(curChannel, WIFI_SECOND_CHAN_NONE);
#endif

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
#if defined(ESP8266)
    wifi_set_channel(curChannel);
#else
    esp_wifi_set_channel(curChannel, WIFI_SECOND_CHAN_NONE);
#endif
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
