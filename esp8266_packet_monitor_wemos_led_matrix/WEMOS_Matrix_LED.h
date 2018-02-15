
#ifndef __WEMOS_Matrix_LED_H
#define __WEMOS_Matrix_LED_H


#if ARDUINO >= 100
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif
#ifndef _swap_int16_t
#define _swap_int16_t(a, b) { int16_t t = a; a = b; b = t; }
#endif


class MLED
{
	public:
		MLED(uint8_t _intensity=4, byte dataPin=D7, byte clockPin=D5);
		void display();
		void clear();
		void dot(uint8_t x, uint8_t y, bool draw=1);
        void drawline(int16_t x0, int16_t y0, int16_t x1, int16_t y1);
        void drawline2(int x0, int y0, int x1, int y1);


	    volatile uint8_t disBuffer[8]={0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	    uint8_t intensity;
	    

	protected:
		void sendCommand(byte led);
	    void sendData(byte add, byte data);
	    void send(byte data);

	    byte dataPin;
	    byte clockPin;

	    

};


#endif

