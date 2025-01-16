# RP2040-UI_Meter

An extremely simple voltage/current meter made of [EETree 12-Finger-Probe](https://www.eetree.cn/platform/2585).

## Hardware
The `12-Finger-Probe` is a development board made by `EETree`,which has the following features:
- MCU: RP2040, a dual-core ARM Cortex-M0+ processor with 264KB of SRAM
- Flash memory: 2MB SPI NOR Flash
- Display: 240x240 ST7789 LCD
- USB-C interface for power supply and programming/debugging
- A 12-pin connector for power output and 9 GPIOs(including 3 ADC channels)

## Software
The software is developed with PlatformIO, thanks for earlephilhower's [arduino-pico core](https://github.com/earlephilhower/arduino-pico)