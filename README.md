BitBank SSD1327+SSD1322 OLED library<br>
Project started 6/10/2019<br>
Copyright (c) 2019 BitBank Software, Inc.<br>
Written by Larry Bank<br>
bitbank@pobox.com<br>
<br>
![SSD1327](/demo.jpg?raw=true "SSD1327")
<br>
The purpose of this code is to easily control SSD1327 and SSD1322 OLED displays. For now,
The code assumes that the SSD1327 is 128x128x4-bpp and SSD1322 is 256x64x4-bpp. Until
I get other displays to test, these are the only sizes which will work with this code.<br>
The code supports driving either display with SPI, I2C and bit
bang connections using any 2 GPIO pins. Instead of adding support for these displays
to a more generalized library, I chose to create stand alone code that could be
as small as possible since the 4-bit gray pixels need to be managed differently
than the 1-bit pixels of bitonal OLEDs and the 16-bit pixels of color LCDs.<br>

This code makes use of my BitBang_I2C library and it must be installed as well.<br>
https://github.com/bitbank2/BitBang_I2C<br>

Features:<br>
---------<br>
- 3 sizes of fixed fonts (6x8, 8x8, 16x16) with background and foreground color<br>
- Optional 8K RAM backing buffer for more advanced features<br>
- Native line, rectangle and ellipse functions (optionally filled)<br>
- Contrast (aka brightness) control<br>
- Set display inverted or flipped 180 degrees<br>
- Draw text with any foreground or background 'color'<br>
- Draw transparent text which doesn't overwrite the background (backing RAM required)<br>
- Rotate and draw bitmaps and transparent patterns<br>
- Light enough to run on an ATtiny85<br> 
<br>

If you find this code useful, please consider buying me a cup of coffee

[![paypal](https://www.paypalobjects.com/en_US/i/btn/btn_donateCC_LG.gif)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=SR4F44J2UR8S4)

