#include <ssd1327.h>

void setup() {
  // addr, flip, invert, SDA, SCL, speed
  // set SDA/SCL to 0xff if using hardware I2C
  // otherwise use the Arduino pin number of Port+bit as a hex number
  // For more speed on AVR micros use port/bit notation (e.g. PORTB bit 2 --> 0xb2)
  ssd1327Init(0x3c, 0, 0, 0xc0, 0xc2, 400000L);
  ssd1327Fill(0);
}

void loop() {
int i;

// Draw 3 sizes of text and use the pixel function to draw an X in the middle
  for (i=0; i<16; i++)
  {
      ssd1327WriteString(0,i*8,(char *)"8x8", FONT_NORMAL, i, 15-i);
      ssd1327WriteString(26,i*8,(char *)"6x8", FONT_SMALL, 15-i, i);
      ssd1327WriteString(46, (i & 0xfe) * 8, (char *)"16x16", FONT_LARGE, 15-i, i);
  }
  for (i=0; i<128; i++)
  {
    ssd1327SetPixel(i/2, i, i & 15);
    ssd1327SetPixel(63-(i/2), i, 15 - (i & 15));
  }
  // Pulse contrast between darkest (0) and lightest (255)
  while (1)
  {
    for (i=0; i<255; i++)
    {
      ssd1327SetContrast(i);
      delay(10);
    }
    for (i=255; i>=0; i--)
    {
      ssd1327SetContrast(i);
      delay(10);
    }
  }
}
