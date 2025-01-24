#include <lvgl.h>
#include <TFT_eSPI.h>
#include <ulog.h>

#include "Display.h"

namespace Display
{
    static TFT_eSPI screen;
    std::function<void(uint32_t *, bool *)> readKeyEventCb;

    static void flushDisplay(lv_display_t *disp, const lv_area_t *area,
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

    static void readKey(lv_indev_t *indev, lv_indev_data_t *data)
    {
        if (!readKeyEventCb)
        {
            ULOG_WARNING("readKeyEventCb not set");
            return;
        }
        bool pressed;
        readKeyEventCb(&data->key, &pressed);
        data->state = pressed ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
    }

    void setReadKeyEventCb(std::function<void(uint32_t *, bool *)> cb)
    {
        readKeyEventCb = cb;
    }

    static void printLog(lv_log_level_t level, const char *buf)
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

        static uint32_t drawBuf[TFT_WIDTH * TFT_HEIGHT / 10];
        auto disp = lv_display_create(TFT_WIDTH, TFT_HEIGHT);
        lv_display_set_flush_cb(disp, flushDisplay);
        lv_display_set_buffers(disp, drawBuf, NULL, sizeof(drawBuf),
                               LV_DISPLAY_RENDER_MODE_PARTIAL);

        lv_indev_t *indev = lv_indev_create();
        lv_indev_set_type(indev, LV_INDEV_TYPE_KEYPAD);
        lv_indev_set_read_cb(indev, readKey);

        auto *label = lv_label_create(lv_screen_active());
        lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
        lv_label_set_text(label, "Fuck my ass!!!!");
    }

    void handleTimer()
    {
        delay(lv_timer_handler());
    }
} // namespace display
