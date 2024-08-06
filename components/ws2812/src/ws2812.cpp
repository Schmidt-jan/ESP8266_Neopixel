#include "ws2812.hpp"

/**
 * @brief Create a new pixel strip. The pin is activated as output, and
 * each pixel is initialized to black (off). The brightness is set to 255.
 *
 * @param pin to which the strip is connected
 * @param numPixels number of LEDs in the strip
 * @param order defines the strip and the color order. For more details see the PixelOrder enum.
 */
WS2812::WS2812(gpio_num_t pin, uint16_t numPixels, PixelOrder::PixelOrder order)
    : pin(pin),
      numPixels(numPixels),
      offW(order >> 9 & 0b111),
      offR(order >> 6 & 0b111),
      offG(order >> 3 & 0b111),
      offB(order & 0b111),
      numLedsPerPixel(3),
      brightness(255),
      pixels(std::vector<uint8_t>(numPixels * numLedsPerPixel)),
      lastShow(RtosTimestamp())
{
    enablePin(pin);
}

void WS2812::enablePin(gpio_num_t pin) const
{
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = 1ULL << pin;
    io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);
}

void WS2812::disablePin(gpio_num_t pin) const
{
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = 1ULL << pin;
    io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);
}

/**
 * @brief Destructor for the WS2812 class.
 * Cleans up the memory allocated for the pixels and sets the pin mode to input.
 *
 */
WS2812::~WS2812(void)
{
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = 1ULL << pin;
    io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);
}

/**
 * @brief Set the color of the whole strip to black (off).
 *
 */
void WS2812::clear()
{
    fill(RgbColor(0, 0, 0));
}

/**
 * @brief Write the rgb color to the pin.
 *
 * As the ccount register is reset by the ccompare interrupt, some aspects have to be considered.
 * The ccount register only counts up to the defined interrupt frequency (normally 100ms). At that
 * point the ccompare interrupt is triggered which resets the ccount register
 * (https://github.com/espressif/ESP8266_RTOS_SDK/issues/750#issuecomment-549261167).
 * As the ccount register is important for the time critical transmission of the data to the strip,
 * an overflow check has to be made. This corrects the timing.
 * However, as no context switch should occur during the transmission, the transmission is wrapped
 * in a critical section. You can control up to which priority interrupts should be handled
 * by setting the configMAX_SYSCALL_INTERRUPT_PRIORITY and configMAX_API_CALL_INTERRUPT_PRIORITY.
 * For more details see the FreeRTOS documentation (https://freertos.org/taskENTER_CRITICAL_taskEXIT_CRITICAL.html,
 * https://freertos.org/a00110.html#kernel_priority)
 *
 *
 * @return
 */
IRAM_ATTR bool WS2812::show(void)
{
    if (!isReady())
    {
        return false;
    }

    uint8_t mask = 0x80;
    uint32_t t, time0 = CYCLES_800_T0H, time1 = CYCLES_800_T1H, period = CYCLES_800, startTime = 0, c;
    uint32_t pinMask = 1ULL << pin; // Assume 'pin' is defined elsewhere

    taskENTER_CRITICAL();
    for (auto it = pixels.cbegin(); it != pixels.cend(); ++it)
    {
        uint8_t pix = (*it) * brightness >> 8;
        for (int bit = 0; bit < 8; ++bit)
        {
            t = (pix & mask) ? time1 : time0;
            while (((c = xthal_get_ccount()) - startTime) < period)
                ;
            GPIO_REG_WRITE(GPIO_OUT_W1TS_ADDRESS, pinMask);
            startTime = c;
            while (((xthal_get_ccount()) - startTime) < t)
                ;
            GPIO_REG_WRITE(GPIO_OUT_W1TC_ADDRESS, pinMask);

            mask >>= 1;
            if (!mask)
            {
                mask = 0x80;
            }
        }
    }

    // Ensure the final bit period is complete
    while ((xthal_get_ccount() - startTime) < period)
        ;

    taskEXIT_CRITICAL();

    lastShow.update();
    return true;
}

IRAM_ATTR bool WS2812::isReady() const
{
    return lastShow.tickDiff() > CYCLES_RESET;
}

/**
 * @brief Fill the whole strip with the given color.
 *
 * @param color
 */
void WS2812::fill(const RgbColor& color)
{
    assert(numLedsPerPixel == 3);

    for (int i = 0; i < numPixels * numLedsPerPixel; i += numLedsPerPixel)
    {
        pixels.at(i + offR) = color.r;
        pixels.at(i + offG) = color.g;
        pixels.at(i + offB) = color.b;
    }
}

/**
 * @brief Set the color to the nth-Pixel
 *
 * @param num (index) of the pixel whose color you want to change.
 * The first pixel has index 0, last pixel has numPixels - 1.
 * @param color an rgb-color
 */
void WS2812::setPixelColor(uint16_t num, const RgbColor& color)
{
    assert(numLedsPerPixel == 3);
    if (num >= numPixels)
        return;

    uint32_t pixIdx = num * numLedsPerPixel;

    pixels.at(pixIdx + offR) = color.r;
    pixels.at(pixIdx + offG) = color.g;
    pixels.at(pixIdx + offB) = color.b;
}

/**
 * @brief Set a brightness value between 0 and 255. To new value is applied to
 * all pixels.
 * @param brightness
 */
void WS2812::setBrightness(uint8_t brightness)
{
    this->brightness = brightness;
}
