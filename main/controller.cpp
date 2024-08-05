#include "controller.hpp"

#define UPDATE_TRANSITION_COLOR(color, target) \
    if (color < target)                        \
    {                                          \
        color++;                               \
    }                                          \
    else if (color > target)                   \
    {                                          \
        color--;                               \
    }

#define UPDATE_RAINBOW_CYCLE()

const char *Controller::TAG = "Controller";

Controller::Controller(WS2812 *led) : led(led)
{
    effect = SOLID;
    effectSpeed = 50;
    currentColor = {0, 0, 0};
    targetColor = {0, 0, 0};
    currentBrightness = 255;
    targetBrightness = 255;
    inTransition = false;
    latestUpdateShown = false;

    led->fill(currentColor);
    led->show();
}

Controller::~Controller()
{
}

void Controller::loop()
{
    // head to the target values befor the effect is shown
    if (inTransition)
    {
        UPDATE_TRANSITION_COLOR(currentColor.r, targetColor.r);
        UPDATE_TRANSITION_COLOR(currentColor.g, targetColor.g);
        UPDATE_TRANSITION_COLOR(currentColor.b, targetColor.b);
        latestUpdateShown = false;
    }

    setEffectPixels();

    // ESP_LOGI(Controller::TAG, "Phase: %d, Sign: %d, Red: %d, Green: %d, Blue: %d", phase, sign, currentColor.r, currentColor.g, currentColor.b);

    if (!latestUpdateShown && led->isReady())
    {
        led->show();
        latestUpdateShown = true;
    }
    else
    {
        latestUpdateShown = false;
    }
}

void Controller::nextRainbowColor(uint8_t *colorMask, int8_t *sign, RgbColor *target, uint8_t offset)
{
    uint8_t* colorPtr = reinterpret_cast<uint8_t*>(target);
    int16_t newVal = (colorPtr[*colorMask] + *sign * offset);
    uint8_t prevColorMask = (*colorMask + 1) % 3;
    uint8_t nextColorMask = (*colorMask + 2) % 3;
    
    if (newVal < 0) {
        colorPtr[*colorMask] = 0;
        colorPtr[nextColorMask] -= newVal;
    } else if (newVal > 255) {
        colorPtr[*colorMask] = 255;
        colorPtr[nextColorMask] -= newVal % 255;
    } else {
        colorPtr[*colorMask] = newVal;
    }

    
    if (colorPtr[*colorMask] == 0 || colorPtr[*colorMask] == 255) {
        *colorMask = (*colorMask + 2) % 3;
        *sign = -*sign;
    }
}

void Controller::setEffectPixels()
{
    switch (effect)
    {
    case SOLID:
        if (inTransition)
        {
            led->fill(currentColor);
            latestUpdateShown = false;
            inTransition = currentColor != targetColor;
        }
        break;
    case RAINBOW:
        if (inTransition)
        {
            inTransition = currentColor != targetColor;
        } else {
            nextRainbowColor(&phase, &sign, &currentColor, 1);
        }
        led->fill(currentColor);
        latestUpdateShown = false;
        break;
    case RAINBOW_CYCLE:
        if (inTransition)
        {
            inTransition = currentColor != targetColor;
            led->fill(currentColor);
        } else {
            nextRainbowColor(&phase, &sign, &currentColor, 1);
            led->setPixelColor(0, currentColor);

            // clone the current color
            uint8_t nextPhase = this->phase;
            int8_t nextSign = this->sign;
            RgbColor nextPixel = RgbColor{currentColor.r, currentColor.g, currentColor.b};
            for (uint16_t i = 1; i < led->getPixelCount(); i++)
            {
                nextRainbowColor(&nextPhase, &nextSign, &nextPixel, 100);
                led->setPixelColor(i, nextPixel);
            }
        }
        latestUpdateShown = false;
        break;
    default:
        ESP_LOGI(Controller::TAG, "Unimplemented effect set");
    }
}

void Controller::setEffect(Effect effect)
{
    this->effect = effect;
    inTransition = true;
    latestUpdateShown = false;

    // set variables for the effect
    switch (effect)
    {
        case RAINBOW:
        case RAINBOW_CYCLE:
            targetColor = {255, 0, 0};
            phase = 1;
            sign = 1;
            break;
        default:
            break;
    }
}

void Controller::setEffectSpeed(uint8_t effectSpeed)
{
    this->effectSpeed = effectSpeed;
}

void Controller::setTargetColor(RgbColor targetColor)
{
    this->targetColor = targetColor;
    inTransition = true;
}

void Controller::setTargetBrightness(uint8_t targetBrightness)
{
    this->targetBrightness = targetBrightness;
    inTransition = true;
}
