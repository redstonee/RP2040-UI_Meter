#pragma once
#include <functional>
#include <cstdint>

namespace Display
{
    void init();
    void handleTimer();
    void setReadKeyEventCb(std::function<void(uint32_t *, bool *)> cb);

    /** Predefined keys to control focused object via lv_group_send(group, c) */
    enum
    {
        LV_KEY_ENTER = 10, /*0x0A, '\n'*/
        LV_KEY_NEXT = 9,   /*0x09, '\t'*/
        LV_KEY_PREV = 11,  /*0x0B, '*/
    };
} // namespace display