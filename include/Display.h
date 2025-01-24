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
     * @brief Handle the timer for the display module
     *
     * Should be called in a loop
     */
    void handleTimer();

    /**
     * @brief Set the Read Key Event Cb object
     *
     * @param cb The callback function to read key events
     */
    void setReadKeyEventCb(ReadKeyEventCallback);

    /** Predefined keys to control focused object via lv_group_send(group, c) */
    enum
    {
        LV_KEY_ENTER = 10, /*0x0A, '\n'*/
        LV_KEY_NEXT = 9,   /*0x09, '\t'*/
        LV_KEY_PREV = 11,  /*0x0B, '*/
    };
} // namespace display