#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"

#define F_CPU (CONFIG_ESP8266_DEFAULT_CPU_FREQ_MHZ * 1000000)

class RtosTimestamp {
    public:
        RtosTimestamp() : rtosTicks(xTaskGetTickCount()), cycleCount(xthal_get_ccount()) {}
        RtosTimestamp(uint32_t rtosTicks, uint32_t cycleCount) : rtosTicks(rtosTicks), cycleCount(cycleCount) {}

        void update() {
            rtosTicks = xTaskGetTickCount();
            cycleCount = xthal_get_ccount();
        }

        uint64_t tickDiff() const {
            uint32_t rtosTicksNow = xTaskGetTickCount();
            uint32_t cycleCountNow = xthal_get_ccount();

            return (uint64_t) (rtosTicksNow - rtosTicks) * (F_CPU / configTICK_RATE_HZ) + cycleCountNow - cycleCount;
        }

    private:
        uint32_t rtosTicks;
        uint32_t cycleCount;
};