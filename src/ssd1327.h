#ifndef __SSD1327__
#define __SSD1327__
#define FONT_NORMAL 0
#define FONT_LARGE 1
#define FONT_SMALL 2

//
// Initializes the OLED controller into "page mode" on I2C
// If SDAPin and SCLPin are not -1, then bit bang I2C on those pins
// Otherwise use the Wire library
//
void ssd1327Init(int iAddr, int bFlip, int bInvert, int iSDAPin, int iSCLPin, int32_t iSpeed);
//
// Initialize an SPI version of the display
//
void ssd1327SPIInit(int iDC, int iCS, int iReset, int bFlip, int bInvert, int32_t iSpeed);
//
// Sets the brightness (0=off, 255=brightest)
//
void ssd1327SetContrast(unsigned char ucContrast);

//
// Sends a command to turn off the OLED display
//
void ssd1327Shutdown();
//
// Power up/down the display
// useful for low power situations
//
void ssd1327Power(byte bOn);
//
// Draw a string of normal (8x8), small (6x8) or large (16x32) characters
// At the given col+row with the given foreground (MSN) and background (LSN) colors
//
void ssd1327WriteString(uint8_t x, uint8_t y, char *szMsg, uint8_t iSize, uint8_t ucFGColor, uint8_t ucBGColor);
//
// Fill the frame buffer with a color
// e.g. black (0x00) or white (0xf)
//
void ssd1327Fill(unsigned char ucColor);
//
// Set an individual pixel to a specific color
// A compromise to avoid needing an 8K back buffer
// A pair of pixels (a byte) is set
// The 128x128 display is remapped as 64x128
//
void ssd1327SetPixel(int x, int y, unsigned char ucColor);
#endif // __SSD1327__
