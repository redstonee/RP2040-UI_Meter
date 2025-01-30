#pragma once
#include <functional>
#include <cstdint>

namespace Display
{
    using ReadKeyEventCallback = std::function<std::pair<uint32_t, bool>()>;
    /**
     * @brief Initialize the display module
     *
     */
    void init();

    /**
     * @brief Main loop function for the display module
     *
     * Should be called in a loop
     */
    void run();

    /**
     * @brief Set the Read Key Event Cb object
     *
     * @param cb The callback function to read key events
     */
    void setReadKeyEventCb(ReadKeyEventCallback);

    /**
     * @brief Update the voltage value on the display
     *
     * @param valid A flag to indicate if the value is valid
     * @param value The voltage value in volts, or INFINITY for overload
     */
    void updateVoltage(const bool, const float);

    /**
     * @brief Update the current value on the display
     *
     * @param valid A flag to indicate if the value is valid
     * @param value The current value in amperes, or INFINITY for overload
     */
    void updateCurrent(const bool, const float);

    /** Predefined keys to control focused object via lv_group_send(group, c) */
    enum
    {
        LV_KEY_ENTER = 10, /*0x0A, '\n'*/
        LV_KEY_NEXT = 9,   /*0x09, '\t'*/
        LV_KEY_PREV = 11,  /*0x0B, '*/
    };
} // namespace display