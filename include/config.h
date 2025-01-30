#pragma once

// Key pins
constexpr auto KEY_M_PIN = 5;
constexpr auto KEY_S_PIN = 6;
constexpr auto KEY_L_PIN = 7;
constexpr auto KEY_OK_PIN = 8;
constexpr auto KEY_R_PIN = 9;

// ADC pins
constexpr auto USENSE_PIN = 26;
constexpr auto ISENSE_PIN = 27;

// Amplifier scale selection pins
constexpr auto U_SCALE0_PIN = 21;
constexpr auto U_SCALE1_PIN = 22;
constexpr auto I_SCALE0_PIN = 23;
constexpr auto I_SCALE1_PIN = 24;

// Display pins are defined in the project configuration file


// Default gain values
constexpr float U_SCALE_DEF_GAINS[] = {0.23, 0.45, 1, 2.14};
constexpr float I_SCALE_DEF_GAIN[] = {5, 10, 22, 47};
constexpr float I_SAMPLE_RES = 0.5; // Sample resistor value in Ohms

// Threshold values for each scale
constexpr float U_SCALE_MAX_VALUE[] = {14, 6.5, 3.1, 1.3};
constexpr float U_SCALE_MIN_VALUE[] = {6, 2.8, 1.1, 0};
constexpr float I_SCALE_MAX_VALUE[] = {1.3, 0.6, 0.25, 0.12};
constexpr float I_SCALE_MIN_VALUE[] = {0.5, 0.2, 0.1, 0};

// Some loop period in ms
constexpr auto SAMPLE_PERIOD = 10;
constexpr auto GET_VALUE_PERIOD = 500;
constexpr auto LVGL_HANDLE_PERIOD = 5;