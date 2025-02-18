#include <lvgl.h>
#include <TFT_eSPI.h>
#include <ulog.h>

#include "Display.h"

namespace Display
{
    static TFT_eSPI screen;
    ReadKeyEventCallback readKeyEventCb;

    static lv_obj_t *vValueLabel;
    static lv_obj_t *iValueLabel;

    // Display update flags
    static bool voltageUpdated = false;
    static bool currentUpdated = false;
    static float voltageValue = 0;
    static float currentValue = 0;

    inline void flushDisplay(lv_display_t *disp, const lv_area_t *area,
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

    inline void readKey(lv_indev_t *indev, lv_indev_data_t *data)
    {
        if (!readKeyEventCb)
        {
            ULOG_WARNING("readKeyEventCb not set");
            return;
        }
        auto [key, pressed] = readKeyEventCb();
        data->key = key;
        data->state = pressed ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
    }

    void setReadKeyEventCb(ReadKeyEventCallback cb)
    {
        readKeyEventCb = cb;
    }

    inline void printLog(lv_log_level_t level, const char *buf)
    {
        constexpr ulog_level_t ULOG_LEVELS[]{
            ULOG_TRACE_LEVEL, ULOG_INFO_LEVEL, ULOG_WARNING_LEVEL,
            ULOG_ERROR_LEVEL, ULOG_ALWAYS_LEVEL, static_cast<ulog_level_t>(0)};

        ulog_message(ULOG_LEVELS[level], buf);
    }

    void init()
    {
        // Initialize the driver and the graphics library
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

        auto keyPadIndev = lv_indev_create();
        lv_indev_set_type(keyPadIndev, LV_INDEV_TYPE_KEYPAD);
        lv_indev_set_read_cb(keyPadIndev, readKey);

        // Put the widgets on the screen
        auto vHintLabel = lv_label_create(lv_screen_active());
        auto iHintLabel = lv_label_create(lv_screen_active());
        lv_obj_align(vHintLabel, LV_ALIGN_LEFT_MID, 8, -60);
        lv_obj_align(iHintLabel, LV_ALIGN_LEFT_MID, 8, 60);
        lv_label_set_text(vHintLabel, "Voltage: ");
        lv_label_set_text(iHintLabel, "Current: ");
        lv_obj_set_style_text_font(vHintLabel, &lv_font_montserrat_24, LV_PART_MAIN);
        lv_obj_set_style_text_font(iHintLabel, &lv_font_montserrat_24, LV_PART_MAIN);

        vValueLabel = lv_label_create(lv_screen_active());
        iValueLabel = lv_label_create(lv_screen_active());
        lv_obj_align(vValueLabel, LV_ALIGN_RIGHT_MID, -8, -60);
        lv_obj_align(iValueLabel, LV_ALIGN_RIGHT_MID, -8, 60);
        lv_obj_set_style_text_font(vValueLabel, &lv_font_montserrat_24, LV_PART_MAIN);
        lv_obj_set_style_text_font(iValueLabel, &lv_font_montserrat_24, LV_PART_MAIN);
        lv_label_set_text(vValueLabel, "---");
        lv_label_set_text(iValueLabel, "---");
    }

    void updateVoltage(const float value)
    {
        voltageValue = value;
        voltageUpdated = true;
    }

    void updateCurrent(const float value)
    {
        currentValue = value;
        currentUpdated = true;
    }

    inline void updateText(lv_obj_t *label, const float value, const char unit)
    {
        if (!label)
        {
            ULOG_ERROR("Cannot update text: target label is NULL");
            return;
        }
        String txt;
        if (value == -1)
            txt = "----";
        else if (value == INFINITY)
            txt = "Overload";
        else
            txt = String(value, 2) + " " + unit;

        lv_label_set_text(label, txt.c_str());
    }

    void run()
    {
        if (voltageUpdated)
        {
            updateText(vValueLabel, voltageValue, 'V');
            voltageUpdated = false;
        }

        if (currentUpdated)
        {
            updateText(iValueLabel, currentValue, 'A');
            currentUpdated = false;
        }

        delay(lv_timer_handler());
    }
} // namespace display
