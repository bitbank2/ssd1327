BitBank SSD1327 OLED library<br>
Project started 6/10/2019<br>
Copyright (c) 2019 BitBank Software, Inc.<br>
Written by Larry Bank<br>
bitbank@pobox.com<br>
<br>
![SSD1327](/demo.jpg?raw=true "SSD1327")
<br>
The purpose of this code is to easily control SSD1327 128x128 OLED
displays using a minimum of FLASH and RAM. The code supports SPI, I2C and bit
bang connections using any 2 GPIO pins. Instead of adding support for this display
to a more generalized library, I chose to create stand alone code that could be
as small as possible.<br>

This code makes use of my BitBang_I2C library and it must be installed as well.<br>
https://github.com/bitbank2/BitBang_I2C<br>

Features:<br>
---------<br>
- 3 sizes of fixed fonts (6x8, 8x8, 16x16) with background and foreground color<br>
- Avoids requiring an 8K RAM backing buffer
- Pixel drawing which uses pixel pairs (64x128 virtual display size)
- Contrast (aka brightness) control
- Set display inverted or flipped 180 degrees
- Light enough to run on an ATtiny85<br> 
<br>

If you find this code useful, please consider buying me a cup of coffee

[![paypal](https://www.paypalobjects.com/en_US/i/btn/btn_donateCC_LG.gif)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=SR4F44J2UR8S4)

