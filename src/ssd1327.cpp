//
// SSD1327 - 128x128x16 grayscale OLED library
// Copyright 2019 (C) BitBank Software, Inc. 
// Project started 6/10/2019
// Written by Larry Bank (bitbank@pobox.com)
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
#ifdef _LINUX_
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <math.h>
#define PROGMEM
#define memcpy_P memcpy
static int file_i2c = 0;
#define USE_BACKBUFFER
#else // Arduino

#include <Arduino.h>
#ifdef __AVR__
#include <avr/pgmspace.h>
#else
// On systems with enough RAM (not AVR), we allocate a static 8K back buffer
#define USE_BACKBUFFER
#endif
#include <BitBang_I2C.h>
#ifndef __AVR_ATtiny85__
#include <Wire.h>
#include <SPI.h>
#endif
#endif // _LINUX_
#include <ssd1327.h>

#ifdef USE_BACKBUFFER
static uint8_t ucBackbuffer[8192];
#endif

void ssd1327Power(unsigned char bOn);

// small (8x8) font
const unsigned char ucFont[]PROGMEM = {
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x06,0x5f,0x5f,0x06,0x00,0x00,
0x00,0x07,0x07,0x00,0x07,0x07,0x00,0x00,0x14,0x7f,0x7f,0x14,0x7f,0x7f,0x14,0x00,
  0x24,0x2e,0x2a,0x6b,0x6b,0x3a,0x12,0x00,0x46,0x66,0x30,0x18,0x0c,0x66,0x62,0x00,
  0x30,0x7a,0x4f,0x5d,0x37,0x7a,0x48,0x00,0x00,0x04,0x07,0x03,0x00,0x00,0x00,0x00,
  0x00,0x1c,0x3e,0x63,0x41,0x00,0x00,0x00,0x00,0x41,0x63,0x3e,0x1c,0x00,0x00,0x00,
  0x08,0x2a,0x3e,0x1c,0x1c,0x3e,0x2a,0x08,0x00,0x08,0x08,0x3e,0x3e,0x08,0x08,0x00,
  0x00,0x00,0x80,0xe0,0x60,0x00,0x00,0x00,0x00,0x08,0x08,0x08,0x08,0x08,0x08,0x00,
  0x00,0x00,0x00,0x60,0x60,0x00,0x00,0x00,0x60,0x30,0x18,0x0c,0x06,0x03,0x01,0x00,
  0x3e,0x7f,0x59,0x4d,0x47,0x7f,0x3e,0x00,0x40,0x42,0x7f,0x7f,0x40,0x40,0x00,0x00,
  0x62,0x73,0x59,0x49,0x6f,0x66,0x00,0x00,0x22,0x63,0x49,0x49,0x7f,0x36,0x00,0x00,
  0x18,0x1c,0x16,0x53,0x7f,0x7f,0x50,0x00,0x27,0x67,0x45,0x45,0x7d,0x39,0x00,0x00,
  0x3c,0x7e,0x4b,0x49,0x79,0x30,0x00,0x00,0x03,0x03,0x71,0x79,0x0f,0x07,0x00,0x00,
  0x36,0x7f,0x49,0x49,0x7f,0x36,0x00,0x00,0x06,0x4f,0x49,0x69,0x3f,0x1e,0x00,0x00,
  0x00,0x00,0x00,0x66,0x66,0x00,0x00,0x00,0x00,0x00,0x80,0xe6,0x66,0x00,0x00,0x00,
  0x08,0x1c,0x36,0x63,0x41,0x00,0x00,0x00,0x00,0x14,0x14,0x14,0x14,0x14,0x14,0x00,
  0x00,0x41,0x63,0x36,0x1c,0x08,0x00,0x00,0x00,0x02,0x03,0x59,0x5d,0x07,0x02,0x00,
  0x3e,0x7f,0x41,0x5d,0x5d,0x5f,0x0e,0x00,0x7c,0x7e,0x13,0x13,0x7e,0x7c,0x00,0x00,
  0x41,0x7f,0x7f,0x49,0x49,0x7f,0x36,0x00,0x1c,0x3e,0x63,0x41,0x41,0x63,0x22,0x00,
  0x41,0x7f,0x7f,0x41,0x63,0x3e,0x1c,0x00,0x41,0x7f,0x7f,0x49,0x5d,0x41,0x63,0x00,
  0x41,0x7f,0x7f,0x49,0x1d,0x01,0x03,0x00,0x1c,0x3e,0x63,0x41,0x51,0x33,0x72,0x00,
  0x7f,0x7f,0x08,0x08,0x7f,0x7f,0x00,0x00,0x00,0x41,0x7f,0x7f,0x41,0x00,0x00,0x00,
  0x30,0x70,0x40,0x41,0x7f,0x3f,0x01,0x00,0x41,0x7f,0x7f,0x08,0x1c,0x77,0x63,0x00,
  0x41,0x7f,0x7f,0x41,0x40,0x60,0x70,0x00,0x7f,0x7f,0x0e,0x1c,0x0e,0x7f,0x7f,0x00,
  0x7f,0x7f,0x06,0x0c,0x18,0x7f,0x7f,0x00,0x1c,0x3e,0x63,0x41,0x63,0x3e,0x1c,0x00,
  0x41,0x7f,0x7f,0x49,0x09,0x0f,0x06,0x00,0x1e,0x3f,0x21,0x31,0x61,0x7f,0x5e,0x00,
  0x41,0x7f,0x7f,0x09,0x19,0x7f,0x66,0x00,0x26,0x6f,0x4d,0x49,0x59,0x73,0x32,0x00,
  0x03,0x41,0x7f,0x7f,0x41,0x03,0x00,0x00,0x7f,0x7f,0x40,0x40,0x7f,0x7f,0x00,0x00,
  0x1f,0x3f,0x60,0x60,0x3f,0x1f,0x00,0x00,0x3f,0x7f,0x60,0x30,0x60,0x7f,0x3f,0x00,
  0x63,0x77,0x1c,0x08,0x1c,0x77,0x63,0x00,0x07,0x4f,0x78,0x78,0x4f,0x07,0x00,0x00,
  0x47,0x63,0x71,0x59,0x4d,0x67,0x73,0x00,0x00,0x7f,0x7f,0x41,0x41,0x00,0x00,0x00,
  0x01,0x03,0x06,0x0c,0x18,0x30,0x60,0x00,0x00,0x41,0x41,0x7f,0x7f,0x00,0x00,0x00,
  0x08,0x0c,0x06,0x03,0x06,0x0c,0x08,0x00,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
  0x00,0x00,0x03,0x07,0x04,0x00,0x00,0x00,0x20,0x74,0x54,0x54,0x3c,0x78,0x40,0x00,
  0x41,0x7f,0x3f,0x48,0x48,0x78,0x30,0x00,0x38,0x7c,0x44,0x44,0x6c,0x28,0x00,0x00,
  0x30,0x78,0x48,0x49,0x3f,0x7f,0x40,0x00,0x38,0x7c,0x54,0x54,0x5c,0x18,0x00,0x00,
  0x48,0x7e,0x7f,0x49,0x03,0x06,0x00,0x00,0x98,0xbc,0xa4,0xa4,0xf8,0x7c,0x04,0x00,
  0x41,0x7f,0x7f,0x08,0x04,0x7c,0x78,0x00,0x00,0x44,0x7d,0x7d,0x40,0x00,0x00,0x00,
  0x60,0xe0,0x80,0x84,0xfd,0x7d,0x00,0x00,0x41,0x7f,0x7f,0x10,0x38,0x6c,0x44,0x00,
  0x00,0x41,0x7f,0x7f,0x40,0x00,0x00,0x00,0x7c,0x7c,0x18,0x78,0x1c,0x7c,0x78,0x00,
  0x7c,0x78,0x04,0x04,0x7c,0x78,0x00,0x00,0x38,0x7c,0x44,0x44,0x7c,0x38,0x00,0x00,
  0x84,0xfc,0xf8,0xa4,0x24,0x3c,0x18,0x00,0x18,0x3c,0x24,0xa4,0xf8,0xfc,0x84,0x00,
  0x44,0x7c,0x78,0x4c,0x04,0x0c,0x18,0x00,0x48,0x5c,0x54,0x74,0x64,0x24,0x00,0x00,
  0x04,0x04,0x3e,0x7f,0x44,0x24,0x00,0x00,0x3c,0x7c,0x40,0x40,0x3c,0x7c,0x40,0x00,
  0x1c,0x3c,0x60,0x60,0x3c,0x1c,0x00,0x00,0x3c,0x7c,0x60,0x30,0x60,0x7c,0x3c,0x00,
  0x44,0x6c,0x38,0x10,0x38,0x6c,0x44,0x00,0x9c,0xbc,0xa0,0xa0,0xfc,0x7c,0x00,0x00,
  0x4c,0x64,0x74,0x5c,0x4c,0x64,0x00,0x00,0x08,0x08,0x3e,0x77,0x41,0x41,0x00,0x00,
  0x00,0x00,0x00,0x77,0x77,0x00,0x00,0x00,0x41,0x41,0x77,0x3e,0x08,0x08,0x00,0x00,
  0x02,0x03,0x01,0x03,0x02,0x03,0x01,0x00,0x70,0x78,0x4c,0x46,0x4c,0x78,0x70,0x00};
  // 5x7 font (in 6x8 cell)
const unsigned char ucSmallFont[]PROGMEM = {
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x06,0x5f,0x06,0x00,0x00,0x07,0x03,0x00,
0x07,0x03,0x00,0x24,0x7e,0x24,0x7e,0x24,0x00,0x24,0x2b,0x6a,0x12,0x00,0x00,0x63,
0x13,0x08,0x64,0x63,0x00,0x36,0x49,0x56,0x20,0x50,0x00,0x00,0x07,0x03,0x00,0x00,
0x00,0x00,0x3e,0x41,0x00,0x00,0x00,0x00,0x41,0x3e,0x00,0x00,0x00,0x08,0x3e,0x1c,
  0x3e,0x08,0x00,0x08,0x08,0x3e,0x08,0x08,0x00,0x00,0xe0,0x60,0x00,0x00,0x00,0x08,
  0x08,0x08,0x08,0x08,0x00,0x00,0x60,0x60,0x00,0x00,0x00,0x20,0x10,0x08,0x04,0x02,
  0x00,0x3e,0x51,0x49,0x45,0x3e,0x00,0x00,0x42,0x7f,0x40,0x00,0x00,0x62,0x51,0x49,
  0x49,0x46,0x00,0x22,0x49,0x49,0x49,0x36,0x00,0x18,0x14,0x12,0x7f,0x10,0x00,0x2f,
  0x49,0x49,0x49,0x31,0x00,0x3c,0x4a,0x49,0x49,0x30,0x00,0x01,0x71,0x09,0x05,0x03,
  0x00,0x36,0x49,0x49,0x49,0x36,0x00,0x06,0x49,0x49,0x29,0x1e,0x00,0x00,0x6c,0x6c,
  0x00,0x00,0x00,0x00,0xec,0x6c,0x00,0x00,0x00,0x08,0x14,0x22,0x41,0x00,0x00,0x24,
  0x24,0x24,0x24,0x24,0x00,0x00,0x41,0x22,0x14,0x08,0x00,0x02,0x01,0x59,0x09,0x06,
  0x00,0x3e,0x41,0x5d,0x55,0x1e,0x00,0x7e,0x11,0x11,0x11,0x7e,0x00,0x7f,0x49,0x49,
  0x49,0x36,0x00,0x3e,0x41,0x41,0x41,0x22,0x00,0x7f,0x41,0x41,0x41,0x3e,0x00,0x7f,
  0x49,0x49,0x49,0x41,0x00,0x7f,0x09,0x09,0x09,0x01,0x00,0x3e,0x41,0x49,0x49,0x7a,
  0x00,0x7f,0x08,0x08,0x08,0x7f,0x00,0x00,0x41,0x7f,0x41,0x00,0x00,0x30,0x40,0x40,
  0x40,0x3f,0x00,0x7f,0x08,0x14,0x22,0x41,0x00,0x7f,0x40,0x40,0x40,0x40,0x00,0x7f,
  0x02,0x04,0x02,0x7f,0x00,0x7f,0x02,0x04,0x08,0x7f,0x00,0x3e,0x41,0x41,0x41,0x3e,
  0x00,0x7f,0x09,0x09,0x09,0x06,0x00,0x3e,0x41,0x51,0x21,0x5e,0x00,0x7f,0x09,0x09,
  0x19,0x66,0x00,0x26,0x49,0x49,0x49,0x32,0x00,0x01,0x01,0x7f,0x01,0x01,0x00,0x3f,
  0x40,0x40,0x40,0x3f,0x00,0x1f,0x20,0x40,0x20,0x1f,0x00,0x3f,0x40,0x3c,0x40,0x3f,
  0x00,0x63,0x14,0x08,0x14,0x63,0x00,0x07,0x08,0x70,0x08,0x07,0x00,0x71,0x49,0x45,
  0x43,0x00,0x00,0x00,0x7f,0x41,0x41,0x00,0x00,0x02,0x04,0x08,0x10,0x20,0x00,0x00,
  0x41,0x41,0x7f,0x00,0x00,0x04,0x02,0x01,0x02,0x04,0x00,0x80,0x80,0x80,0x80,0x80,
  0x00,0x00,0x03,0x07,0x00,0x00,0x00,0x20,0x54,0x54,0x54,0x78,0x00,0x7f,0x44,0x44,
  0x44,0x38,0x00,0x38,0x44,0x44,0x44,0x28,0x00,0x38,0x44,0x44,0x44,0x7f,0x00,0x38,
  0x54,0x54,0x54,0x08,0x00,0x08,0x7e,0x09,0x09,0x00,0x00,0x18,0xa4,0xa4,0xa4,0x7c,
  0x00,0x7f,0x04,0x04,0x78,0x00,0x00,0x00,0x00,0x7d,0x40,0x00,0x00,0x40,0x80,0x84,
  0x7d,0x00,0x00,0x7f,0x10,0x28,0x44,0x00,0x00,0x00,0x00,0x7f,0x40,0x00,0x00,0x7c,
  0x04,0x18,0x04,0x78,0x00,0x7c,0x04,0x04,0x78,0x00,0x00,0x38,0x44,0x44,0x44,0x38,
  0x00,0xfc,0x44,0x44,0x44,0x38,0x00,0x38,0x44,0x44,0x44,0xfc,0x00,0x44,0x78,0x44,
  0x04,0x08,0x00,0x08,0x54,0x54,0x54,0x20,0x00,0x04,0x3e,0x44,0x24,0x00,0x00,0x3c,
  0x40,0x20,0x7c,0x00,0x00,0x1c,0x20,0x40,0x20,0x1c,0x00,0x3c,0x60,0x30,0x60,0x3c,
  0x00,0x6c,0x10,0x10,0x6c,0x00,0x00,0x9c,0xa0,0x60,0x3c,0x00,0x00,0x64,0x54,0x54,
  0x4c,0x00,0x00,0x08,0x3e,0x41,0x41,0x00,0x00,0x00,0x00,0x77,0x00,0x00,0x00,0x00,
  0x41,0x41,0x3e,0x08,0x00,0x02,0x01,0x02,0x01,0x00,0x00,0x3c,0x26,0x23,0x26,0x3c};

// some globals
static int iCSPin, iDCPin, iResetPin;
static int iMaxX, iMaxY, iPitch;
static int oled_type, oled_flip, oled_addr;
static int iSDAPin, iSCLPin;
static void ssd1327WriteCommand(unsigned char c);

// wrapper/adapter functions to make the code work on Linux
#ifdef _LINUX_
static uint8_t pgm_read_byte(uint8_t *ptr)
{
  return *ptr;
}
static int16_t pgm_read_word(uint8_t *ptr)
{
  return ptr[0] + (ptr[1]<<8);
}
int I2CReadRegister(uint8_t addr, uint8_t reg, uint8_t *pBuf, int iLen)
{
int rc;
  rc = write(file_i2c, &reg, 1);
  rc = read(file_i2c, pBuf, iLen);
  return (rc > 0);
}
int I2CInit(int iSDAPin, int iSCLPin, int32_t iSpeed)
{
char filename[32];

  sprintf(filename, "/dev/i2c-%d", iSDAPin); // I2C bus number passed in SDA pin
  if ((file_i2c = open(filename, O_RDWR)) < 0)
     return 1;
  if (ioctl(file_i2c, I2C_SLAVE, iSCLPin) < 0) // set slave address
  {
     close(file_i2c);
     file_i2c = 0;
     return 1;
  }
  return 0;
}
static void I2CWrite(unsigned char ucAddr, unsigned char *pData, int iLen)
{
  write(file_i2c, pData, iLen);
}
#endif // _LINUX_

// use only the bitbang version on ATtiny85 to avoid linking wire library
#if defined( __AVR_ATtiny85__ ) || defined(_LINUX_)
static void oledWrite(unsigned char *pData, int iLen)
{
  I2CWrite(oled_addr, pData, iLen);
} /* oledWrite() */
#else
// Wrapper function to write I2C data on Arduino
static void oledWrite(unsigned char *pData, int iLen)
{
  if (iCSPin != -1) // we're writing to SPI, treat it differently
  {
    digitalWrite(iDCPin, (pData[0] == 0) ? LOW : HIGH); // data versus command
    digitalWrite(iCSPin, LOW);
#ifdef HAL_ESP32_HAL_H_ 
   {
   uint8_t ucTemp[1024];
        SPI.transferBytes(&pData[1], ucTemp, iLen-1);
   }
#else
    SPI.transfer(&pData[1], iLen-1);
#endif
    digitalWrite(iCSPin, HIGH);
  }
  else // must be I2C
  {
    if (iSDAPin != -1 && iSCLPin != -1)
    {
       I2CWrite(oled_addr, pData, iLen);
    }
    else
    {
       Wire.beginTransmission(oled_addr);
       Wire.write(pData, iLen);
       Wire.endTransmission();
    }
  } // I2C
} /* oledWrite() */
#endif // !__AVR_ATtiny85__

//
// Turn off the display
//
void ssd1327Shutdown(void)
{
	ssd1327Power(0); // power off the display
#ifdef  _LINUX_
	close(file_i2c);
	file_i2c = 0;	
#endif

} /* ssd1327Shutdown() */
//
// ssd1322
//
static uint8_t ssd1322_init_table[] = {
	2, 0xfd, 0x12, // unlock the controller
        1, 0xa4, // set display off
//        1, 0xb9, // default grayscale mode
	2, 0xb3, 0x91, // set clock divider
	2, 0xca, 0x3f, // set COMS multiplex ratio 1/64
	2, 0xa2, 0x00, // set display offset  
	2, 0xa1, 0x00, // set display start line
	3, 0xa0, 0x14, 0x11, // set display remap
	2, 0xb5, 0x00, // disable GPIO
	2, 0xab, 0x01, // select external VDD regulator (none)
	3, 0xb4, 0xa0, 0xfd, // external VSL display enhancement
	2, 0xb1, 0xe2, // set phase length
	3, 0xd1, 0x82, 0x20, // display enahancement B
	1, 0xa6, // set normal display mode
	0
};
void ssd1322Init(int bFlip)
{
uint8_t *s = (uint8_t *)ssd1322_init_table;
uint8_t ucTemp[8], iCount;

   iCount = 1;
   if (bFlip)
     ssd1322_init_table[19] = 0x06;
   else
     ssd1322_init_table[19] = 0x14; // segment remap
   ucTemp[0] = 0x40; // pretend it's I2C data
   while (iCount)
   {
     iCount = *s++;
     if (iCount != 0)
     {
       ssd1327WriteCommand(*s++);
       memcpy(&ucTemp[1], s, iCount-1);
       oledWrite(ucTemp, iCount);
       s += iCount-1;
     }
   }
	
} /* ssd1322Init() */

#if !defined( __AVR_ATtiny85__ ) && !defined(_LINUX_)
//
// Initialize the OLED controller for SPI mode
//
void ssd1327SPIInit(int iType, int iDC, int iCS, int iReset, int bFlip, int bInvert, int32_t iSpeed)
{
uint8_t uc[32];

  iDCPin = iDC;
  iCSPin = iCS;
  oled_type = iType;
  iResetPin = iReset;
  oled_flip = bFlip;

  pinMode(iDCPin, OUTPUT);
  pinMode(iCSPin, OUTPUT);
  digitalWrite(iCSPin, HIGH);

  // Reset it
  if (iResetPin != -1)
  {
    pinMode(iResetPin, OUTPUT); 
    digitalWrite(iResetPin, HIGH);
    delay(50);
    digitalWrite(iResetPin, LOW);
    delay(50);
    digitalWrite(iResetPin, HIGH);
    delay(10);
  }
// Initialize SPI
  SPI.begin();
  SPI.beginTransaction(SPISettings(iSpeed, MSBFIRST, SPI_MODE0));
//  SPI.setClockDivider(16);
//  SPI.setBitOrder(MSBFIRST);
//  SPI.setDataMode(SPI_MODE0);

  if (oled_type == OLED_256x64)
  {
    iMaxX = 256;
    iMaxY = 64;
    iPitch = 128;
    ssd1322Init(bFlip);
  }
  else
  {
    iPitch = 64;
    iMaxX = 128;
    iMaxY = 128;
  }

  ssd1327Power(1);

  if (bInvert)
  {
    uc[0] = 0; // command
    uc[1] = 0xa7; // invert command
    oledWrite(uc, 2);
  }
  if (bFlip && oled_type == OLED_128x128) // rotate display 180
  {
    uc[0] = 0; // command
    uc[1] = 0xa0;
    oledWrite(uc, 2);
    uc[0] = 0;
    uc[1] = 0xc0;
    oledWrite(uc, 2);
  }

} /* ssd1327SPIInit() */
#endif
//
// Initializes the OLED controller
//
void ssd1327Init(int iType, int iAddr, int bFlip, int bInvert, int sda, int scl, int32_t iSpeed)
{
unsigned char uc[4];

  oled_addr = iAddr;
  oled_flip = bFlip;
  oled_type = iType;

  iSDAPin = sda;
  iSCLPin = scl;
// Disable SPI mode code
  iCSPin = iDCPin = iResetPin = -1;

#ifdef _LINUX_
  I2CInit(sda, iAddr, iSpeed); // bus number and address for Linux
#else
if (sda != -1 && scl != -1)
{
  I2CInit(sda, scl, iSpeed);
}
#ifndef __AVR_ATtiny85__
else
{
  Wire.begin(); // Initiate the Wire library
  Wire.setClock(iSpeed); // use high speed I2C mode (default is 100Khz)
}
#endif
#endif // _LINUX_

  if (oled_type == OLED_256x64)
  {
    iMaxX = 256;
    iMaxY = 64;
    iPitch = 128;
    ssd1322Init(bFlip);
  }
  else
  {
    iPitch = 64;
    iMaxX = 128;
    iMaxY = 128;
  }

  ssd1327Power(0); // turn off the power
  if (oled_type == OLED_128x128)
  {
    uc[0] = 0x00; // command
    uc[1] = 0xa0; // GDDRAM mapping
    if (bFlip)
       uc[2] = 0x42;
    else
       uc[2] = 0x51; // default (top to bottom, left to right mapping)
    oledWrite(uc, 3);
  }
  ssd1327Power(1); // turn on the power
  uc[0] = 0; // command
  uc[1] = (bInvert) ? 0xa7:0xa4; // invert command / normal display
  oledWrite(uc, 2);
} /* ssd1327Init() */
//
// Sends a command to turn on or off the OLED display
//
void ssd1327Power(unsigned char bOn)
{
    if (bOn)
      ssd1327WriteCommand(0xaf); // turn on OLED
    else
      ssd1327WriteCommand(0xae); // turn off OLED
} /* oledPower() */

// Send a single byte command to the OLED controller
static void ssd1327WriteCommand(unsigned char c)
{
unsigned char buf[2];

  buf[0] = 0x00; // command introducer
  buf[1] = c;
  oledWrite(buf, 2);
} /* ssd1327WriteCommand() */

static void ssd1327WriteCommand2(unsigned char c, unsigned char d)
{
unsigned char buf[3];

  buf[0] = 0x00;
  buf[1] = c;
  buf[2] = d;
  oledWrite(buf, 3);
} /* ssd1327WriteCommand2() */

//
// Send commands to position the "cursor" (aka memory write address)
// to the given row and column as well as the ending col/row
//
static void ssd1327SetPosition(int x, int y, int cx, int cy)
{
unsigned char buf[8];

  buf[0] = 0x00; // command introducer
  buf[1] = 0x15; // column start/end
  if (oled_type == OLED_256x64)
  {
    oledWrite(buf, 2);
    buf[0] = 0x40; // data
    buf[1] = 28 + (x/4); // strange SSD1322 mapping
    buf[2] = 28 + (((x+cx)/4)-1);
    oledWrite(buf, 3); // need to write this part as data
    buf[0] = 0x00; // command
    buf[1] = 0x75; // set row
    oledWrite(buf, 2);
    buf[0] = 0x40; // data
    buf[1] = y;
    buf[2] = y+cy-1;
    oledWrite(buf, 3);
    buf[0] = 0x00; // command
    buf[1] = 0x5c; // enable RAM write
    oledWrite(buf, 2);
  }
  else
  {
    buf[2] = x/2; // start address
    buf[3] = (uint8_t)(((x+cx)/2)-1); // end address
    buf[4] = 0x75; // row start/end
    buf[5] = y; // start row
    buf[6] = y+cy-1; // end row
    oledWrite(buf, 7);
  }
} /* ssd1327SetPosition() */

//
// Write a block of pixel data to the OLED
// Length can be anything from 1 to 8192 (whole display)
//
static void ssd1327WriteDataBlock(unsigned char *ucBuf, int iLen)
{
unsigned char ucTemp[129];

  ucTemp[0] = 0x40; // data command
// Copying the data has the benefit in SPI mode of not letting
// the original data get overwritten by the SPI.transfer() function
  memcpy(&ucTemp[1], ucBuf, iLen);
  oledWrite(ucTemp, iLen+1);
} /* ssd1327WriteDataBlock() */

#ifdef FUTURE
//
// Load a 128x64 1-bpp Windows bitmap
// Pass the pointer to the beginning of the BMP file
// First pass version assumes a full screen bitmap
//
int oledLoadBMP(unsigned char *pBMP)
{
int16_t i16;
int iOffBits, q, y, j; // offset to bitmap data
int iPitch;
unsigned char x, z, b, *s;
unsigned char dst_mask;
unsigned char ucTemp[16]; // process 16 bytes at a time
unsigned char bFlipped = false;

  i16 = pgm_read_word(pBMP);
  if (i16 != 0x4d42) // must start with 'BM'
     return -1; // not a BMP file
  i16 = pgm_read_word(pBMP + 18);
  if (i16 != 128) // must be 128 pixels wide
     return -1;
  i16 = pgm_read_word(pBMP + 22);
  if (i16 != 64 && i16 != -64) // must be 64 pixels tall
     return -1;
  if (i16 == 64) // BMP is flipped vertically (typical)
     bFlipped = true;
  i16 = pgm_read_word(pBMP + 28);
  if (i16 != 1) // must be 1 bit per pixel
     return -1;
  iOffBits = pgm_read_word(pBMP + 10);
  iPitch = 16;
  if (bFlipped)
  { 
    iPitch = -16;
    iOffBits += (63 * 16); // start from bottom
  }

// rotate the data and send it to the display
  for (y=0; y<8; y++) // 8 lines of 8 pixels
  {
     ssd1327SetPosition(0, y);
     for (j=0; j<8; j++) // do 8 sections of 16 columns
     {
         s = &pBMP[iOffBits + (j*2) + (y * iPitch*8)]; // source line
         memset(ucTemp, 0, 16); // start with all black
         for (x=0; x<16; x+=8) // do each block of 16x8 pixels
         {
            dst_mask = 1;
            for (q=0; q<8; q++) // gather 8 rows
            {
               b = pgm_read_byte(s + (q * iPitch));
               for (z=0; z<8; z++) // gather up the 8 bits of this column
               {
                  if (b & 0x80)
                      ucTemp[x+z] |= dst_mask;
                  b <<= 1;
               } // for z
               dst_mask <<= 1;
            } // for q
            s++; // next source byte
         } // for x
         ssd1327WriteDataBlock(ucTemp, 16);
     } // for j
  } // for y
  return 0;
} /* oledLoadBMP() */
#endif // FUTURE
//
// Draw a string of normal (8x8), small (6x8) or large (16x32) characters
// At the given col+row
// For AVR systems (very little RAM), the text gets drawn directly on the display
// For non-AVR systems, the text is written to the back buffer and transparent text
// can be enabled by setting the background color to -1
//
void ssd1327WriteString(uint8_t x, uint8_t y, char *szMsg, uint8_t iSize, int ucFG, int ucBG)
{
int i, iFontOff;
int tx, ty;
unsigned char uc, ucMask;
unsigned char c, *s, *d, ucTemp2[8];
#ifndef USE_BACKBUFFER
unsigned char ucTemp[40];
#else
int iBG;
#endif

#ifdef __AVR__
  if (ucBG == -1) ucBG = 0; // no transparent text allowed
#endif
    if (iSize == FONT_NORMAL || iSize == FONT_SMALL) // 8x8 and 6x8 font
    {
       uint8_t cx = (iSize == FONT_NORMAL) ? 8:6;
       uint8_t *pFont = (iSize == FONT_NORMAL) ? (uint8_t*)ucFont:(uint8_t*)ucSmallFont;
       i = 0;
       if (y > iMaxY-7) return; // will write past the bottom
       while (x < iMaxX-7 && szMsg[i] != 0)
       {
         ssd1327SetPosition(x, y, cx, 8);
         c = (unsigned char)szMsg[i];
         iFontOff = (int)(c-32) * cx;
         // we can't directly use the pointer to FLASH memory, so copy to a local buffer
         memcpy_P(ucTemp2, &pFont[iFontOff], cx);
         s = ucTemp2;
#ifdef USE_BACKBUFFER
         d = &ucBackbuffer[(y*iPitch) + (x/2)];
#else
         d = &ucTemp[0];
         *d++ = 0x40; // data introducer
#endif
         ucMask = 1;
         for (ty=0; ty<8; ty++)
         {
           if (ucBG == -1) // transparent text
           {
           for (tx=0; tx<cx; tx+=2)
           {
              if (s[tx] & ucMask) // foreground
              {
                d[0] &= 0xf;
                d[0] |= (ucFG << 4);
              }
              if (s[tx+1] & ucMask)
              {
                d[0] &= 0xf0;
                d[0] |= ucFG;
              }
              d++;
           } // for tx
           }
           else // regular text
           {
           for (tx=0; tx<cx; tx+=2)
           {
              if (s[tx] & ucMask)
                 uc = ucFG << 4;
              else
                 uc = ucBG << 4;
              if (s[tx+1] & ucMask)
                 uc |= ucFG;
              else
                 uc |= ucBG;
              *d++ = uc; // store pixel pair
           } // for tx
           }
           ucMask <<= 1;
#ifdef USE_BACKBUFFER
           d -= cx/2;
           d += iPitch; // move to next line
#endif
         } // for ty
#ifndef USE_BACKBUFFER
         oledWrite(ucTemp, 1+(cx*4)); // write character pattern
#endif
         x += cx;
         i++;
       }
    }
    else if (iSize == FONT_LARGE) // 16x16 font
    {
      if (y > iMaxY-15) return; // will write past the bottom
      i = 0;
      while (x < iMaxX-15 && szMsg[i] != 0)
      {
// stretch the 'normal' font instead of using the big font
          int tx, ty;
          c = szMsg[i] - 32;
          unsigned char ucMask;
          s = (unsigned char *)&ucFont[(int)c*8];
          memcpy_P(ucTemp2, s, 8);
          iBG = ucBG;
          ucFG |= (ucFG << 4); // 2 pixels at a time
          ucBG |= (ucBG << 4);
          // Stretch the font to double width + double height
          ucMask = 1;
#ifdef USE_BACKBUFFER
          d = &ucBackbuffer[(y*iPitch)+(x/2)];
#else
          ssd1327SetPosition(x, y, 16, 16);
          ucTemp[0] = 0x40; // start of data (write one row at a time)
#endif
          for (ty=0; ty<8; ty++)
          {
              for (tx=0; tx<8; tx++)
              {
#ifdef USE_BACKBUFFER
                  if (ucTemp2[tx] & ucMask) // pixel set
                     d[tx] = d[tx+iPitch] = ucFG;
                  else if (ucBG != -1)
                     d[tx] = d[tx+iPitch] = ucBG;
#else
                  if (ucTemp2[tx] & ucMask) // pixel set
                     c = ucFG;
                  else
                     c = ucBG;
                  ucTemp[1+tx] = ucTemp[1+tx+8] = c; // double it vertically
#endif
              }
#ifdef USE_BACKBUFFER
              d += iPitch*2; // move to next line
#else
              oledWrite(ucTemp, 17); // write 2 rows of the character
#endif
              ucMask <<= 1;
          }
          x += 16;
          i++;
       }
    }
} /* ssd1327WriteString() */
#ifdef USE_BACKBUFFER
//
// For drawing ellipses, a circle is drawn and the x and y pixels are scaled by a 16-bit integer fraction
// This function draws a single pixel and scales its position based on the x/y fraction of the ellipse
//
void DrawScaledPixel(int32_t iCX, int32_t iCY, int32_t x, int32_t y, int32_t iXFrac, int32_t iYFrac, uint8_t ucColor)
{   
uint8_t *d;

    if (iXFrac != 0x10000) x = (x * iXFrac) >> 16;
    if (iYFrac != 0x10000) y = (y * iYFrac) >> 16;
    x += iCX; y += iCY;
    if (x < 0 || x >= iMaxX || y < 0 || y >= iMaxY)
        return; // off the screen
    d = &ucBackbuffer[(y * iPitch) + x/2];
    if (x & 1)
    {
      d[0] &= 0xf0;
      d[0] |= ucColor;
    }
    else
    {
      d[0] &= 0xf;
      d[0] |= (ucColor << 4);
    }
} /* DrawScaledPixel() */
void DrawScaledLine(int32_t iCX, int32_t iCY, int32_t x, int32_t y, int32_t iXFrac, int32_t iYFrac, uint8_t ucColor)
{   
int32_t iLen, x2;
uint8_t *d;

    if (iXFrac != 0x10000) x = (x * iXFrac) >> 16;
    if (iYFrac != 0x10000) y = (y * iYFrac) >> 16;
    iLen = x * 2;
    x = iCX - x; y += iCY;
    x2 = x + iLen; 
    if (y < 0 || y >= iMaxY) 
        return; // completely off the screen
    if (x < 0) x = 0;
    if (x2 >= iMaxX) x2 = iMaxX-1;
    iLen = x2 - x + 1; // new length
    d = &ucBackbuffer[(y * iPitch) + x/2];
    uint8_t c = ucColor | (ucColor << 4);
    if (x & 1) // starting on odd pixel
    {
      d[0] &= 0xf0;
      d[0] |= ucColor;
      d++;
      iLen--;
    }
    while (iLen >= 2) // middle part
    {
      *d++ = c;
      iLen -= 2;
    }
    if (iLen) // last odd pixel
    {
      d[0] &= 0xf;
      d[0] |= (ucColor << 4);
    }
} /* DrawScaledLine() */
//
// Draw the 8 pixels around the Bresenham circle
// (scaled to make an ellipse)
//
void BresenhamCircle(int32_t iCX, int32_t iCY, int32_t x, int32_t y, int32_t iXFrac, int32_t iYFrac, uint8_t iColor, int bFill)
{
    if (bFill) // draw a filled ellipse
    {
        // for a filled ellipse, draw 4 lines instead of 8 pixels
        DrawScaledLine(iCX, iCY, x, y, iXFrac, iYFrac, iColor);
        DrawScaledLine(iCX, iCY, x, -y, iXFrac, iYFrac, iColor);
        DrawScaledLine(iCX, iCY, y, x, iXFrac, iYFrac, iColor);
        DrawScaledLine(iCX, iCY, y, -x, iXFrac, iYFrac, iColor);
    }
    else // draw 8 pixels around the edges
    {
        DrawScaledPixel(iCX, iCY, x, y, iXFrac, iYFrac, iColor);
        DrawScaledPixel(iCX, iCY, -x, y, iXFrac, iYFrac, iColor);
        DrawScaledPixel(iCX, iCY, x, -y, iXFrac, iYFrac, iColor);
        DrawScaledPixel(iCX, iCY, -x, -y, iXFrac, iYFrac, iColor);
        DrawScaledPixel(iCX, iCY, y, x, iXFrac, iYFrac, iColor);
        DrawScaledPixel(iCX, iCY, -y, x, iXFrac, iYFrac, iColor);
        DrawScaledPixel(iCX, iCY, y, -x, iXFrac, iYFrac, iColor);
        DrawScaledPixel(iCX, iCY, -y, -x, iXFrac, iYFrac, iColor);
    }
} /* BresenhamCircle() */
void ssd1327Ellipse(int32_t iCenterX, int32_t iCenterY, int32_t iRadiusX, int32_t iRadiusY, uint8_t ucColor, int bFilled)
{
    int32_t iRadius, iXFrac, iYFrac;
    int32_t iDelta, x, y;

    if (iRadiusX > iRadiusY) // use X as the primary radius
    {   
        iRadius = iRadiusX;
        iXFrac = 65536;
        iYFrac = (iRadiusY * 65536) / iRadiusX;
    }
    else
    {
        iRadius = iRadiusY;
        iXFrac = (iRadiusX * 65536) / iRadiusY;
        iYFrac = 65536;
    }
    // set up a buffer with the widest possible run of pixels to dump in 1 shot
    iDelta = 3 - (2 * iRadius);
    x = 0; y = iRadius;
    while (x <= y)
    {
        BresenhamCircle(iCenterX, iCenterY, x, y, iXFrac, iYFrac, ucColor, bFilled);
        x++;
        if (iDelta < 0)
        {
            iDelta += (4*x) + 6;
        }
        else
        {
            iDelta += 4 * (x-y) + 10;
            y--;
        }
    }
} /* ssd1327Ellipse() */
//
// Draw a 1-bpp pattern into the backbuffer with the given color
// 1 bits are drawn as color, 0 are transparent
//
void ssd1327DrawPattern(uint8_t *pPattern, int iSrcPitch, int iDestX, int iDestY, int iCX, int iCY, uint8_t ucColor)
{
    int x, y;
    uint8_t *s, uc, ucPixel, ucMask;
    uint8_t *d;
   
    if (iDestX + iCX > iMaxX || iDestY + iCY > iMaxY || iDestX < 0 || iDestY < 0 || iCX < 0 || iCY < 0)
        return; // invalid parameter
   
    if (iDestX+iCX > iMaxX) // trim to fit on display
        iCX = (iMaxX - iDestX);
    if (iDestY+iCY > iMaxY)
        iCY = (iMaxY - iDestY);
    if (pPattern == NULL || iDestX < 0 || iDestY < 0 || iCX <=0 || iCY <= 0)
        return;
    for (y=0; y<iCY; y++)
    {
        d = &ucBackbuffer[((iDestY+y)*iPitch) + (iDestX/2)];
        s = &pPattern[y * iSrcPitch];
        ucMask = uc = 0;
        for (x=0; x<iCX; x+=2)
        {
            ucMask >>= 1;
            ucPixel = d[0];
            if (ucMask == 0)
            {
                ucMask = 0x80;
                uc = *s++;
            }
            if (uc & ucMask) // active pixel
            {
                ucPixel &= 0xf;
                ucPixel |= (ucColor << 4); 
            }
            ucMask >>= 1;
            if (uc & ucMask) // odd pixel
            {
                ucPixel &= 0xf0;
                ucPixel |= ucColor;
            }
            *d++ = ucPixel; // write modified pixel pair
        } // for x
    } // for y

} /* ssd1327DrawPattern() */

//
// Display part or whole of the backbuffer or a custom bitmap to the visible display
// Pass a NULL pointer to display the backbuffer
//
void ssd1327ShowBitmap(uint8_t *pBuffer, int iLocalPitch, int x, int y, int w, int h)
{
int ty;
uint8_t *s;

  if (x < 0 || y < 0 || x >= iMaxX || y >= iMaxY || (x+w) > iMaxX || (y+h) > iMaxY)
    return; // invalid coordinates
  if (pBuffer == NULL)
  {
    pBuffer = &ucBackbuffer[(y*iPitch)+(x/2)]; // starting point also
    iLocalPitch = iPitch;
  }
  ssd1327SetPosition(x, y, w, h);
  for (ty=0; ty<h; ty++)
  {
    s = &pBuffer[iLocalPitch * (y+ty)];
    ssd1327WriteDataBlock(&s[x/2], w/2);
  } // for y

} /* ssd1327ShowBitmap() */
// Set an individual pixel to a specific color
// Only affects the backbuffer and must be explicitly
// displayed later with ssd1327ShowBuffer()
void ssd1327SetPixel(int x, int y, unsigned char ucColor)
{
uint8_t c, *d;
  d = &ucBackbuffer[(y*iPitch)+x/2];
  c = d[0];
  if (x & 1)
  {
    c &= 0xf0; // right pixel
    c |= ucColor; 
  }
  else
  {
    c &= 0xf;
    c |= (ucColor << 4);
  }
  d[0] = c;
} /* ssd1327SetPixel() */

#endif // USE_BACKBUFFER
//
// Sets the brightness (0=off, 255=brightest)
//
void ssd1327SetContrast(unsigned char ucContrast)
{
  ssd1327WriteCommand2(0x81, ucContrast);
} /* ssd1327SetContrast() */

//
// Fill the frame buffer with a byte pattern
// e.g. all off (0x00) or all on (0xff)
//
void ssd1327Fill(unsigned char ucColor)
{
uint8_t x, y;
unsigned char temp[16];

  ucColor |= (ucColor << 4); // set pixel pair color
  memset(temp, ucColor, 16);
#ifdef USE_BACKBUFFER
  memset(ucBackbuffer, ucColor, sizeof(ucBackbuffer));
#endif
 
  ssd1327SetPosition(0,0,iMaxX,iMaxY);
  for (y=0; y<iMaxY; y++)
  {
    for (x=0; x<iMaxX/32; x++)
    {
      ssd1327WriteDataBlock(temp, 16); 
    } // for x
  } // for y
} /* ssd1327Fill() */

#ifdef USE_BACKBUFFER
void ssd1327Rectangle(int x, int y, int w, int h, uint8_t ucColor, int bFill)
{
int i;
uint8_t *d;

  ucColor |= (ucColor << 4); // left/right in a byte
  // check bounds
  if (x < 0 || x >= iMaxX || x+w > iMaxX)
     return; // out of bounds
  if (y < 0 || y >= iMaxY || y+h > iMaxY)
     return;

  if (bFill)
  {
    for (i=0; i<h; i++)
    {
      d = &ucBackbuffer[(y+i)*iPitch + (x/2)];
      memset(d, ucColor, w/2);
    } // for i
  } // filled
  else // outline
  {
    // draw top/bottom
    d = &ucBackbuffer[y*iPitch + (x/2)];
    memset(d, ucColor, w/2);
    d = &ucBackbuffer[(y+h-1)*iPitch + (x/2)];
    memset(d, ucColor, w/2);
    // draw left/right
    ucColor &= 0xf;
    d = &ucBackbuffer[(y*iPitch)+(x/2)];
    for (i=0; i<h; i++)
    {
      d[0] &= 0xf; // set left pixel
      d[0] |= (ucColor << 4);
      d[w/2-1] &= 0xf0; // set right pixel
      d[w/2-1] |= ucColor;
      d += iPitch;
    }
  } // outline
} /* ssd1327Rectangle() */

uint8_t *ssd1327GetBackbuffer(void)
{
  return ucBackbuffer;
}
//
// Table of sine values for 0-360 degrees expressed as a signed 16-bit value
// from -32768 (-1) to 32767 (1)
//
int16_t i16SineTable[] = {0,572, 1144, 1715, 2286, 2856, 3425, 3993, 4560, 5126,  // 0-9
        5690,  6252, 6813, 7371, 7927, 8481, 9032, 9580, 10126, 10668, // 10-19
        11207,  11743, 12275, 12803, 13328, 13848, 14365, 14876, 15384, 15886,// 20-29
        16384,  16877, 17364, 17847, 18324, 18795, 19261, 19720, 20174, 20622,// 30-39
        21063,  21498, 21926, 22348, 22763, 23170, 23571, 23965, 24351, 24730,// 40-49
        25102,  25466, 25822, 26170, 26510, 26842, 27166, 27482, 27789, 28088,// 50-59
        28378,  28660, 28932, 29197, 29452, 29698, 29935, 30163, 30382, 30592,// 60-69
        30792,  30983, 31164, 31336, 31499, 31651, 31795, 31928, 32052, 32166,// 70-79
        32270,  32365, 32440, 32524, 32599, 32643, 32688, 32723, 32748, 32763,// 80-89
        32767,  32763, 32748, 32723, 32688, 32643, 32588, 32524, 32449, 32365,// 90-99
        32270,  32166, 32052, 31928, 31795, 31651, 31499, 31336, 31164, 30983,// 100-109
        30792,  30592, 30382, 30163, 29935, 29698, 29452, 29197, 28932, 28660,// 110-119
        28378,  28088, 27789, 27482, 27166, 26842, 26510, 26170, 25822, 25466,// 120-129
        25102,  24730, 24351, 23965, 23571, 23170, 22763, 22348, 21926, 21498,// 130-139
        21063,  20622, 20174, 19720, 19261, 18795, 18324, 17847, 17364, 16877,// 140-149
        16384,  15886, 15384, 14876, 14365, 13848, 13328, 12803, 12275, 11743,// 150-159
        11207,  10668, 10126, 9580, 9032, 8481, 7927, 7371, 6813, 6252,// 160-169
        5690,  5126, 4560, 3993, 3425, 2856, 2286, 1715, 1144, 572,//  170-179
        0,  -572, -1144, -1715, -2286, -2856, -3425, -3993, -4560, -5126,// 180-189
        -5690,  -6252, -6813, -7371, -7927, -8481, -9032, -9580, -10126, -10668,// 190-199
        -11207,  -11743, -12275, -12803, -13328, -13848, -14365, -14876, -15384, -15886,// 200-209
        -16384,  -16877, -17364, -17847, -18324, -18795, -19261, -19720, -20174, -20622,// 210-219
        -21063,  -21498, -21926, -22348, -22763, -23170, -23571, -23965, -24351, -24730, // 220-229
        -25102,  -25466, -25822, -26170, -26510, -26842, -27166, -27482, -27789, -28088, // 230-239
        -28378,  -28660, -28932, -29196, -29452, -29698, -29935, -30163, -30382, -30592, // 240-249
        -30792,  -30983, -31164, -31336, -31499, -31651, -31795, -31928, -32052, -32166, // 250-259
        -32270,  -32365, -32449, -32524, -32588, -32643, -32688, -32723, -32748, -32763, // 260-269
        -32768,  -32763, -32748, -32723, -32688, -32643, -32588, -32524, -32449, -32365, // 270-279
        -32270,  -32166, -32052, -31928, -31795, -31651, -31499, -31336, -31164, -30983, // 280-289
        -30792,  -30592, -30382, -30163, -29935, -29698, -29452, -29196, -28932, -28660, // 290-299
        -28378,  -28088, -27789, -27482, -27166, -26842, -26510, -26170, -25822, -25466, // 300-309
        -25102,  -24730, -24351, -23965, -23571, -23170, -22763, -22348, -21926, -21498, // 310-319
        -21063,  -20622, -20174, -19720, -19261, -18795, -18234, -17847, -17364, -16877, // 320-329
        -16384,  -15886, -15384, -14876, -14365, -13848, -13328, -12803, -12275, -11743, // 330-339
        -11207,  -10668, -10126, -9580, -9032, -8481, -7927, -7371, -6813, -6252,// 340-349
        -5690,  -5126, -4560, -3993, -3425, -2856, -2286, -1715, -1144, -572, // 350-359
// an extra 90 degrees to simulate the cosine function
        0,572,  1144, 1715, 2286, 2856, 3425, 3993, 4560, 5126,// 0-9
        5690,  6252, 6813, 7371, 7927, 8481, 9032, 9580, 10126, 10668,// 10-19
        11207,  11743, 12275, 12803, 13328, 13848, 14365, 14876, 15384, 15886,// 20-29
        16384,  16877, 17364, 17847, 18324, 18795, 19261, 19720, 20174, 20622,// 30-39
        21063,  21498, 21926, 22348, 22763, 23170, 23571, 23965, 24351, 24730,// 40-49
        25102,  25466, 25822, 26170, 26510, 26842, 27166, 27482, 27789, 28088,// 50-59
        28378,  28660, 28932, 29197, 29452, 29698, 29935, 30163, 30382, 30592,// 60-69
        30792,  30983, 31164, 31336, 31499, 31651, 31795, 31928, 32052, 32166,// 70-79
    32270,  32365, 32440, 32524, 32599, 32643, 32688, 32723, 32748, 32763}; // 80-89


//
// Rotate a 1-bpp or 4-bpp image around a given center point
// valid angles are 0-359
//
void ssd1327RotateBitmap(uint8_t *pSrc, uint8_t *pDest, int iBpp, int iWidth, int iHeight, int iPitch, int iCenterX, int iCenterY, int iAngle)
{
int32_t i, j, x, y;
int16_t pre_sin[512], pre_cos[512], *pSin, *pCos;
int32_t tx, ty, sa, ca;
uint8_t *s, *d, uc, ucMask;

    if (pSrc == NULL || pDest == NULL || iWidth < 2 || iHeight < 1 || iPitch < 1 || iAngle < 0 || iAngle > 359 || iCenterX < 0 || iCenterX >= iWidth || iCenterY < 0 || iCenterY >= iHeight || (iBpp != 1 && iBpp != 4))
        return;
    // since we're rotating from dest back to source, reverse the angle
    iAngle = 360 - iAngle;
    if (iAngle == 360) // just copy src to dest
    {
        memcpy(pDest, pSrc, iHeight * iPitch);
        return;
    }
    // Create a quicker lookup table for sin/cos pre-multiplied at the given angle
    sa = (int32_t)i16SineTable[iAngle]; // sine of given angle
    ca = (int32_t)i16SineTable[iAngle+90]; // cosine of given angle
    for (i=-256; i<256; i++) // create the pre-calc tables
    {
        pre_sin[i+256] = (sa * i) >> 15; // sin * x
        pre_cos[i+256] = (ca * i) >> 15;
    }
    pSin = &pre_sin[256]; pCos = &pre_cos[256]; // point to 0 points in tables
    for (y=0; y<iHeight; y++)
    {
        int16_t siny = pSin[y-iCenterY];
        int16_t cosy = pCos[y-iCenterY];
        int iShift;
        uint8_t ucPixel;
        d = &pDest[y * iPitch];
        ucMask = 0x80; // for 1-bpp
        iShift = 0; // for 4-bpp
        uc = ucPixel = 0;
        for (x=0; x<iWidth; x++)
        {
            // Rotate from the destination pixel back to the source to not have gaps
            // x' = cos*x - sin*y, y' = sin*x + cos*y
            tx = iCenterX + pCos[x-iCenterX] - siny;
            ty = iCenterY + pSin[x-iCenterX] + cosy;
            if (iBpp == 1)
            {
                if (tx > 0 && ty > 0 && tx < iWidth && ty < iHeight) // check source pixel
                {
                    s = &pSrc[(ty*iPitch)+(tx>>3)];
                    if (s[0] & (0x80 >> (tx & 7)))
                        uc |= ucMask; // set destination pixel
                }
                ucMask >>= 1;
                if (ucMask == 0) // write the byte into the destination bitmap
                {
                    ucMask = 0x80;
                    *d++ = uc;
                    uc = 0;
                }
            }
            else // 4-bpp
            {
                if (tx > 0 && ty > 0 && tx < iWidth && ty < iHeight) // check source pixel
                {
                    uc = pSrc[(ty*iPitch)+(tx/2)];
                    if (tx & 1)
                      uc >>= 4;
                    else
                      uc &= 0xf; // isolate the pixel in the lower 4 bits
                    ucPixel |= (uc << iShift);
                    iShift = 4-iShift;
                    if (iShift == 0) // time to store the pair
                    {
                      *d++ = ucPixel;
                      ucPixel = 0;
                    }
                }
            }
        }
        if (iBpp == 1 && ucMask != 0x80) // store partial byte
            *d++ = uc;
        if (iBpp == 4 && iShift == 4) // store partial byte
            *d++ = ucPixel;
    } // for y
} /* ssd1327RotateBitmap() */

void ssd1327DrawLine(int x1, int y1, int x2, int y2, uint8_t ucColor)
{
  int temp;
  int dx = x2 - x1;
  int dy = y2 - y1;
  int error;
  uint8_t *p;
  int xinc, yinc, shift;
  int y, x;
  
  if (x1 < 0 || x2 < 0 || y1 < 0 || y2 < 0 || x1 >= iMaxX || x2 >= iMaxX || y1 >= iMaxY || y2 >= iMaxY)
     return;

  if(abs(dx) > abs(dy)) {
    // X major case
    if(x2 < x1) {
      dx = -dx;
      temp = x1;
      x1 = x2;
      x2 = temp;
      temp = y1;
      y1 = y2;
      y2 = temp;
    }

    y = y1;
    dy = (y2 - y1);
    error = dx >> 1;
    yinc = 1;
    if (dy < 0)
    {
      dy = -dy;
      yinc = -1;
    }
    p = &ucBackbuffer[(x1/2) + (y1 * iPitch)]; // point to current spot in back buffer
    shift = (x1 & 1) ? 0:4; // current bit offset
    for(x=x1; x1 <= x2; x1++) {
      *p &= (0xf0 >> shift);
      *p |= (ucColor << shift);
      shift = 4-shift;
      if (shift == 4) // time to increment pointer
         p++;
      error -= dy;
      if (error < 0)
      {
        error += dx;
        if (yinc > 0)
           p += iPitch;
        else
           p -= iPitch;
        y += yinc;
      }
    } // for x1    
  }
  else {
    // Y major case
    if(y1 > y2) {
      dy = -dy;
      temp = x1;
      x1 = x2;
      x2 = temp;
      temp = y1;
      y1 = y2;
      y2 = temp;
    } 

    p = &ucBackbuffer[(x1/2) + (y1 * iPitch)]; // point to current spot in back buffer
    shift = (x1 & 1) ? 0:4; // current bit offset
    dx = (x2 - x1);
    error = dy >> 1;
    xinc = 1;
    if (dx < 0)
    {
      dx = -dx;
      xinc = -1;
    }
    for(x = x1; y1 <= y2; y1++) {
      *p &= (0xf0 >> shift); // set the pixel
      *p |= (ucColor << shift);
      error -= dx;
      p += iPitch; // y1++
      if (error < 0)
      {
        error += dy;
        x += xinc;
        shift = 4-shift;
        if (xinc == 1)
        {
          if (shift == 4) // time to increment pointer
            p++;
        }
        else
        {
          if (shift == 0)
            p--;
        }
      }
    } // for y
  } // y major case
} /* ssd1327DrawLine() */
#endif // USE_BACKBUFFER
