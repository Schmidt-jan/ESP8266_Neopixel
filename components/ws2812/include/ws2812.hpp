#pragma once

#include <stdint.h>
#include "driver/gpio.h"
#include "PixelOrder.hpp"
#include "FreeRTOS.h"
#include "freertos/task.h"
#include <vector>
#include "rtosTimestamp.hpp"

#define F_CPU (CONFIG_ESP8266_DEFAULT_CPU_FREQ_MHZ * 1000000)
#define CYCLES_800_T0H  (F_CPU / 2500001) // 0.4us
#define CYCLES_800_T1H  (F_CPU / 1250001) // 0.8us
#define CYCLES_800      (F_CPU /  800001) // 1.25us per bit
#define CYCLES_RESET    (F_CPU /  19000) // 50us
#define CYCLES_PER_TICK (F_CPU / configTICK_RATE_HZ)

struct RgbColor {
    uint8_t r;
    uint8_t g;
    uint8_t b;

    RgbColor() : r(0), g(0), b(0) {}
    RgbColor(uint8_t r, uint8_t g, uint8_t b) : r(r), g(g), b(b) {}

    bool operator==(const RgbColor &rhs) const {
        return r == rhs.r && g == rhs.g && b == rhs.b;
    }

    bool operator!=(const RgbColor &rhs) const {
        return !(rhs == *this);
    }
};


/**
 * @brief This class provides basic functions to control a WS2812 LED strip.
 * Theoretically the library should also work on the following platforms:
 * - WS2811
 * - WS2812
 * - WS2812B
 * - WS2813
 * 
 * The library is inspired by the Adafruit NeoPixel library for Arduino, but
 * modified to work in a FreeRTOS environment. Also 
 * 
 */
class WS2812 {
    
public:
    WS2812(gpio_num_t pin, uint16_t numPixels, PixelOrder::PixelOrder pixelOrder);  
    ~WS2812();  
    bool show();
    void clear();
    void fill(const RgbColor&);
    void setPixelColor(uint16_t n, const RgbColor& color);
    void setBrightness(uint8_t);
    bool isReady() const;
    bool stripHasWhite() const;    
    uint8_t getPixelCount() const { return numPixels; }
    

private:
    const gpio_num_t pin;
    uint16_t numPixels;
    const uint8_t offW;
    const uint8_t offR;
    const uint8_t offG;
    const uint8_t offB;
    const uint8_t numLedsPerPixel;
    uint8_t brightness;
    std::vector<uint8_t> pixels;
    RtosTimestamp lastShow;

    void enablePin(gpio_num_t pin) const;
    void disablePin(gpio_num_t pin) const;
};