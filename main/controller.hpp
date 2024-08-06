#pragma once
#include "ws2812.hpp"
#include "esp_log.h"
#include <memory>

enum Effect {
    SOLID = 0,
    RAINBOW,
    RAINBOW_CYCLE,
};

class Controller {
public:
    static const char *TAG;
    Controller(std::unique_ptr<WS2812> led);
    ~Controller();
    void loop(void);
    
    void setEffect(Effect effect);
    void setEffectSpeed(uint8_t effectSpeed);
    void setTargetColor(RgbColor targetColor);
    void setTargetBrightness(uint8_t targetBrightness);

    Effect getEffect() {
        return effect;
    }
    uint8_t getEffectSpeed() {
        return effectSpeed;
    }
    RgbColor getTargetColor() {
        return targetColor;
    }
    uint8_t getTargetBrightness() {
        return targetBrightness;
    }

private:
    std::unique_ptr<WS2812> led;
    Effect effect;
    uint8_t effectSpeed;
    RgbColor currentColor;      // refers to the first pixel (current). All other pixels are calculated from this.
    RgbColor targetColor;       // refers to the first pixel (target). All other pixels are calculated from this.
    uint8_t currentBrightness;
    uint8_t targetBrightness;
    bool inTransition;          // true if the currentColor is not equal to the targetColor
    bool latestUpdateShown;

    void setEffectPixels();

    // RAINBOW variables
    uint8_t phase;
    int8_t sign;
    void nextRainbowColor(uint8_t *phase, int8_t *sign, RgbColor *color, uint8_t offset);
};
