//
// oled test program
// Written by Larry Bank
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "ssd1327.h"

int main(int argc, char *argv[])
{
int iChannel;
int iOLEDAddr = 0x3c; // typical address; it can also be 0x3d
int iOLEDType = OLED_128x128; // Change this for your specific display
int bFlip = 0, bInvert = 0;
int i, x, y, r;
uint8_t c;

	iChannel = 1;
	ssd1327Init(iOLEDType, iOLEDAddr, bFlip, bInvert, iChannel, 0, 0);
	printf("Successfully opened I2C bus %d\n", iChannel);
	ssd1327Fill(0); // fill with black
	ssd1327WriteString(0,0,"SSD1327 Library!",FONT_NORMAL,0xf,0);
	ssd1327WriteString(0,8,"BIG!",FONT_LARGE,0xf,0);
	ssd1327WriteString(0,40,"Small", FONT_SMALL,0xf,0);
        for (i=0; i<100; i++)
	{
	  x = rand(128); y = rand(128); r = rand(64); c = rand(16);
          ssd1327Ellipse(x, y, r, r, c, 1);
	}
	ssd1327ShowBuffer(0,0,128,128);
	printf("Press ENTER to quit\n");
	getchar();
	ssd1327Shutdown();
   return 0;
} /* main() */
