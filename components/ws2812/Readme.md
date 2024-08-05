# WS28XX - ESP8266 FreeRtos
This library provides basic functionalities to control an addressable LED-strip.  

The library was testes on an ESP8266 with the [FreeRtos SDK](https://github.com/espressif/ESP8266_RTOS_SDK) and a WS2812B LED-strip and and GBR Pixel ordering. Both possible CPU speeds of 80 and 160 Mhz were tested.

Theoretically it should be possible to use this library for each pixel color 
ordering, also pixel-strips which support white LEDs.

# Config
There is not much to explain 
``` cpp

```

# Functionalities
The following functionalities are provided:
* `void show()` - transfer all configurations to the strip
* `void clear()` - set all pixels black (off)
* `void fill(RgbColor color)` - set all pixels color
* `void fill(WrgbColor color)` - set all pixels  color
* `void setPixelColor(uint16_t n, RgbColor color)` - set a single pixels color
* `void setPixelColor(uint16_t n, WrgbColor color)` - set a single pixels color
* `void setBrightness(uint8_t)` - set the brightness