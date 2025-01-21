#include <lvgl.h>
#include <TFT_eSPI.h>
#include <ulog.h>

#include "Display.h"

namespace Display
{
    TFT_eSPI screen;
    std::function<void(uint32_t *, bool *)> readKeyEventCb;

    void flushDisplay(lv_display_t *disp, const lv_area_t *area,
                      uint8_t *px_map)
    {
        uint32_t w = lv_area_get_width(area);
        uint32_t h = lv_area_get_height(area);

        screen.startWrite();
        screen.setAddrWindow(area->x1, area->y1, w, h);
        screen.pushPixelsDMA((uint16_t *)px_map, w * h);
        screen.endWrite();

        lv_disp_flush_ready(disp);
    }

    void readKey(lv_indev_t *indev, lv_indev_data_t *data)
    {
        bool pressed;
        readKeyEventCb(&data->key, &pressed);
        data->state = pressed ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
    }

    void setReadKeyEventCb(std::function<void(uint32_t *, bool *)> cb)
    {
        readKeyEventCb = cb;
    }

    void printLog(lv_log_level_t level, const char *buf)
    {
        constexpr ulog_level_t ULOG_LEVELS[]{
            ULOG_TRACE_LEVEL, ULOG_INFO_LEVEL, ULOG_WARNING_LEVEL,
            ULOG_ERROR_LEVEL, ULOG_ALWAYS_LEVEL, static_cast<ulog_level_t>(0)};

        ulog_message(ULOG_LEVELS[level], buf);
    }

    void init()
    {
        screen.begin();
        screen.setSwapBytes(true);
        screen.initDMA();

        lv_init();
        lv_tick_set_cb(millis);

        lv_log_register_print_cb(printLog);

        static uint32_t draw_buf[TFT_WIDTH * TFT_WIDTH / 10];
        auto disp = lv_display_create(TFT_WIDTH, TFT_WIDTH);
        lv_display_set_flush_cb(disp, flushDisplay);
        lv_display_set_buffers(disp, draw_buf, NULL, sizeof(draw_buf),
                               LV_DISPLAY_RENDER_MODE_PARTIAL);

        lv_indev_t *indev = lv_indev_create();
        lv_indev_set_type(indev, LV_INDEV_TYPE_KEYPAD);
        lv_indev_set_read_cb(indev, readKey);
    }

    void handleTimer()
    {
        lv_timer_handler();
    }
} // namespace display
