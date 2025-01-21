#pragma once

// Key pins
constexpr auto KEY_M_PIN = 5;
constexpr auto KEY_S_PIN = 6;
constexpr auto KEY_L_PIN = 7;
constexpr auto KEY_OK_PIN = 8;
constexpr auto KEY_R_PIN = 9;

// ADC pins
constexpr auto VSENSE_PIN = 26;
constexpr auto ISENSE_PIN = 27;

// Amplifier scale selection pins(TBD)
constexpr auto V_SCALE0_PIN = 21;
constexpr auto V_SCALE1_PIN = 22;
constexpr auto I_SCALE0_PIN = 23;
constexpr auto I_SCALE1_PIN = 24;

// Display pins are defined in the project configuration file

// Auto-Scale switching threshold
constexpr auto V_SW_TH_HIGH = 1000;
constexpr auto V_SW_TH_LOW = 100;
constexpr auto I_SW_TH_HIGH = 1000;
constexpr auto I_SW_TH_LOW = 100;

// Some loop period in ms
constexpr auto SAMPLE_PERIOD = 10;
constexpr auto GET_VALUE_PERIOD = 500;
constexpr auto LVGL_HANDLE_PERIOD = 5;