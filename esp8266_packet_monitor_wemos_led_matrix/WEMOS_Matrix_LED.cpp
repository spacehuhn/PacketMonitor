#include "WEMOS_Matrix_LED.h"


MLED::MLED(uint8_t _intensity, byte dataPin, byte clockPin)
{
	this->dataPin = dataPin;
	this->clockPin = clockPin;

	if(_intensity>7)
		intensity=7;
	else
		intensity=_intensity;

	pinMode(dataPin, OUTPUT);
	pinMode(clockPin, OUTPUT);
	
	digitalWrite(dataPin, HIGH);
	digitalWrite(clockPin, HIGH);

}

void MLED::display()
{

	for(uint8_t i=0;i<8;i++)
	{
		sendData(i,disBuffer[i]);

		digitalWrite(dataPin, LOW);
  		digitalWrite(clockPin, LOW);
  		digitalWrite(clockPin, HIGH);
  		digitalWrite(dataPin, HIGH);
	}

	sendCommand(0x88|(intensity));
}

void MLED::clear()
{
	for(uint8_t i=0;i<8;i++)
	{
		disBuffer[i]=0x00;
	}

}

void MLED::dot(uint8_t x, uint8_t y, bool draw)
{
	x&=0x07;
	y&=0x07;

	if(draw)
	{
		disBuffer[y]|=(1<<x);
	}
	else
	{
		disBuffer[y]&=~(1<<x);
	}

}

void MLED::drawline(int16_t x0, int16_t y0, int16_t x1, int16_t y1) {
  int16_t steep = abs(y1 - y0) > abs(x1 - x0);
  if (steep) {
    _swap_int16_t(x0, y0);
    _swap_int16_t(x1, y1);
  }

  if (x0 > x1) {
    _swap_int16_t(x0, x1);
    _swap_int16_t(y0, y1);
  }

  int16_t dx, dy;
  dx = x1 - x0;
  dy = abs(y1 - y0);

  int16_t err = dx / 2;
  int16_t ystep;

  if (y0 < y1) {
    ystep = 1;
  } else {
    ystep = -1;
  }

  for (; x0<=x1; x0++) {
    if (steep) {
      dot(y0, x0);
    } else {
      dot(x0, y0);
    }
    err -= dy;
    if (err < 0) {
      y0 += ystep;
      err += dx;
    }
  }
}

void MLED::drawline2(int x0, int y0, int x1, int y1)
{
  boolean xy_swap=false; 
  if (abs(y1 - y0) > abs(x1 - x0)) {
	    xy_swap = true;
	    int temp = x0;
	    x0 = y0;
	    y0 = temp;
	    temp = x1;
	    x1 = y1;
	    y1 = temp;
	}
	// If line goes from right to left, swap the endpoints
   if (x1 - x0 < 0) {
    int temp = x0;
    x0 = x1;
    x1 = temp;
    temp = y0;
    y0 = y1;
    y1 = temp;
    }

  int x,                   // Current x position
  y = y0,                  // Current y position
  e = 0,                   // Current error
  m_num = y1 - y0,         // Numerator of slope
  m_denom = x1 - x0,       // Denominator of slope
  threshold  = m_denom/2;  // Threshold between E and NE increment 

  for (x = x0; x < x1; x++) {
    if (xy_swap)
	dot(y,x);
    else dot(x,y);
  e += m_num;
    // Deal separately with lines sloping upward and those
    // sloping downward
   if (m_num < 0) {
	if (e < -threshold) {
	    e += m_denom;
	    y--;
	}
    }
    else if (e > threshold) {
	e -= m_denom;
	y++;
    }
  }
  if (xy_swap)
    dot(y,x);
  else dot(x,y);  
}

void MLED::sendCommand(byte cmd)
{
  digitalWrite(dataPin, LOW);
  send(cmd);
  digitalWrite(dataPin, HIGH);
}

void MLED::sendData(byte address, byte data)
{
  sendCommand(0x44);
  digitalWrite(dataPin, LOW);
  send(0xC0 | address);
  send(data);
  digitalWrite(dataPin, HIGH);
}

void MLED::send(byte data)
{
  for (int i = 0; i < 8; i++) {
    digitalWrite(clockPin, LOW);
    digitalWrite(dataPin, data & 1 ? HIGH : LOW);
    data >>= 1;
    digitalWrite(clockPin, HIGH);
  }
}