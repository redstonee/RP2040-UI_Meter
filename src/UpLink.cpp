#include <Arduino.h>
#include <ulog.h>

#include "UpLink.h"
#include "Tools.hpp"

namespace UpLink
{
    void init()
    {
        Serial.begin(115200);
        static auto logOutput = [](ulog_level_t level, char *msg)
        {
            Serial.printf("[%d] ", millis());
            Serial.print(ulog_level_name(level));
            Serial.print(": ");
            Serial.println(msg);
        };
        ulog_subscribe(logOutput, ULOG_DEBUG_LEVEL);
    }

    /**
     * @brief Send the voltage and current values to the up-link application
     *
     * @param uValue The voltage value
     * @param iValue The current value
     */
    void sendValue(const float uValue, const float iValue)
    {
        struct __attribute__((packed)) UpLinkData
        {
            uint8_t header = 0x55;
            float u;
            float i;
            uint8_t checksum;
        } data{
            .u = uValue,
            .i = iValue,
        };

        data.checksum = Tools::calcSum(reinterpret_cast<uint8_t *>(&data), sizeof(data) - 1);
        Serial.write(reinterpret_cast<uint8_t *>(&data), sizeof(data));
    }
} // namespace UpLink
