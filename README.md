# PacketMonitor
ESP8266 + OLED = WiFi Packet Monitor 📦📺

![]()

[![Donate](https://www.paypalobjects.com/en_US/i/btn/btn_donateCC_LG.gif)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=RCHANSVSX9M8C)  


## Introduction

Did you ever asked yourself how many internet traffic is on the air around you?  
This little project is great to see how many packets are just flying around on which channel!

## Building it

**The shopping list:**  
- an ESP8266 (doesn't matter which module or board)
- SSD1306 or SH1106 I2C OLED Display 128x64 pixel
- [Optional] a button (if you use a dev. board with a flash button on it you can use that instead)
- wires to connect everything

You can also buy a ready to use board wich comes with the code preflashed and ready to use!  
- [AliExpress]()
- [tindie]()
(By purchasing you support me and my projects!)


**Connecting the Display:**

| Display | ESP8266 |
| ------- | ------- |
| GND     | GND     |
| VCC     | 3.3V    |
| SDA     | GPIO 5 (D1) |
| SCL     | GPIO 4 (D2) |

If necessary you can modify the I2C pins in the code of course:
```
//create display(Adr, SDA-pin, SCL-pin)
SSD1306 display(0x3c, 5, 4); //GPIO 5 = D1, GPIO 4 = D2
//SH1106 display(0x3c, 5, 4);
```

For the button you can either use the flash button or connect your own.  

**Connecting the button:**
Modify the button pin in the code to whatever pin you like to use:
```
#define btn 0 /* GPIO 0 = FLASH BUTTON */
```
Then connect the button between GND and the button PIN from the code.


**That's all :)**


## How to install it  

**You will only need to follow one of the installation methods!**

### Uploading the .bin file

Upload the `deauth_detector.bin` using the [esptool-gui](https://github.com/Rodmg/esptool-gui), [nodemcu-flasher](https://github.com/nodemcu/nodemcu-flasher) **or** the [esptool](https://github.com/espressif/esptool) from Espressif.

### Using Arduino

**1** Install [Arduino](https://www.arduino.cc/en/Main/Software)  
**2** Install the [ESP8266 SDK](https://github.com/esp8266/Arduino)  
**2** Install the [esp8266-oled-ssd1306](https://github.com/squix78/esp8266-oled-ssd1306) library  
**3** Download this project and open it with Arduino
**4** Maybe customize the code:
```
//===== SETTINGS =====//
//include the library you need
#include "SSD1306.h"
//#include "SH1106.h"

#define btn 0 /* GPIO 0 = FLASH BUTTON */

//create display(Adr, SDA-pin, SCL-pin)
SSD1306 display(0x3c, 5, 4); //GPIO 5 = D1, GPIO 4 = D2
//SH1106 display(0x3c, 5, 4);
  
#define maxCh 13 //max Channel -> US = 11, EU = 13, Japan = 14
#define ledPin 2 //led pin ( 2 = built-in LED)
#define packetRate 5 //min. packets before it gets recognized as an attack

#define flipDisplay true
```
**5** Upload the code to your ESP8266 (don't forget to set it to the right upload settings!)
**6** Disconnect and reconnect your ESP8266 to restart it properly.

**DONE :)**
