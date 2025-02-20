#pragma once

#include <Arduino.h>
#include <utility>
#include <ulog.h>

class VoltMeter
{
private:
    uint32_t adcPin;
    uint32_t scale0Pin;
    uint32_t scale1Pin;

    uint8_t activeScale = 0;
    float scaleGains[4];

    // Sample buffer for voltage smoothing
    static constexpr uint8_t N_SAMPLES = 10;
    uint16_t readBuffer[N_SAMPLES]{0};
    uint8_t bufferFilled = 0;

public:
    VoltMeter(uint32_t adc_pin, uint32_t scale_pin0, uint32_t scale_pin1)
        : adcPin(adc_pin), scale0Pin(scale_pin0), scale1Pin(scale_pin1)
    {
        pinMode(scale0Pin, OUTPUT);
        pinMode(scale1Pin, OUTPUT);
        selectScale(0);
    }

    /**
     * @brief Declare the actual gains for each scale,
     * which should be set before getting correct voltage values
     *
     * @param scale0_gain
     * @param scale1_gain
     * @param scale2_gain
     * @param scale3_gain
     */
    inline void setGains(const float scale0_gain, const float scale1_gain, const float scale2_gain, const float scale3_gain)
    {
        scaleGains[0] = scale0_gain;
        scaleGains[1] = scale1_gain;
        scaleGains[2] = scale2_gain;
        scaleGains[3] = scale3_gain;
    }

    /**
     * @brief Declare the actual gains for each scale,
     *
     * @param gains An array containing 4 gain values in float
     */
    inline void setGains(const float gains[4])
    {
        memcpy(scaleGains, gains, sizeof(scaleGains));
    }

    /**
     * @brief Select the gain of the amplifier
     *
     *  This will clear the buffer
     *
     * @param scale The scale number (0-3)
     */
    inline void selectScale(const uint8_t scale)
    {
        if (scale > 3)
            return;

        digitalWrite(scale0Pin, (scale & 1) ? 1 : 0); // Lower bit
        digitalWrite(scale1Pin, (scale & 2) ? 1 : 0); // Higher bit
        bufferFilled = 0;                             // Needless to actually modify the buffer
        activeScale = scale;
    }

    inline uint8_t getActiveScale()
    {
        return activeScale;
    }

    /**
     * @brief Get a sample from the ADC and push it into the buffer
     *
     * Should be called repeatedly with a constant interval
     */
    void convertOnce()
    {
        // Shift the samples left
        for (uint8_t i = 1; i < N_SAMPLES; i++)
        {
            readBuffer[i - 1] = readBuffer[i];
        }

        readBuffer[N_SAMPLES - 1] = analogRead(adcPin);
        if (bufferFilled < N_SAMPLES)
            bufferFilled++;
    }

    /**
     * @brief Get the raw voltage value from the buffer
     *
     * @return Value in volts
     */
    float getRawVoltage()
    {
        float val = 0;
        for (auto &v : readBuffer)
        {
            val += v;
        }
        val /= N_SAMPLES;
        val *= 3.3 / (1 << ADC_RESOLUTION); // Convert to raw voltage

        return val;
    }

    /**
     * @brief Get the smoothed voltage value from the buffer
     *
     * Only succeed if the buffer is filled
     *
     * @param resolution
     * @return Value in volts, or -1 if it's invalid
     */
    float readVoltage()
    {
        if (bufferFilled < N_SAMPLES)
            return -1;

        auto val = getRawVoltage() / scaleGains[activeScale]; // Apply the gain
        return val;
    }
};