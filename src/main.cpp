#include <Arduino.h>
#include <EEPROM.h>
#include <ulog.h>

#include "Tools.hpp"
#include "Console.h"
#include "Display.h"
#include "KeyPad.hpp"
#include "config.h"
#include "VoltMeter.hpp"

struct MeterSettings
{
  uint8_t header; // Should be 0x69
  float vScaleGains[4];
  float iScaleGains[4];

  uint8_t checksum; // XOR of the payload bytes
};

extern "C"
{
  extern const char help_cal[];
}

/**
 * @brief The entry point of the program
 */
void setup()
{
  Console::init();
  analogReadResolution(ADC_RESOLUTION);

  VoltMeter uMeter(USENSE_PIN, U_SCALE0_PIN, U_SCALE1_PIN);
  VoltMeter iMeter(ISENSE_PIN, I_SCALE0_PIN, I_SCALE1_PIN);

  // Load the settings from "EEPROM"
  MeterSettings settings;
  EEPROM.begin(sizeof(settings));
  EEPROM.get(0, settings);

  auto sum = Tools::calcSum(&settings, sizeof(settings) - 1);
  if (settings.header != 0x69 || sum != settings.checksum)
  {
    ULOG_WARNING("No valid gain settings stored.");
    uMeter.setGains(U_SCALE_DEF_GAINS);
    iMeter.setGains(I_SCALE_DEF_GAIN);
  }
  else
  {
    uMeter.setGains(settings.vScaleGains);
    iMeter.setGains(settings.iScaleGains);
  }

  uint8_t calibrating = 0; // 0: not calibration, 1: voltage, 2: current

  auto cmdCalCallback = [&settings, &calibrating, &uMeter, &iMeter](std::span<String> args)
  {
    // cal start
    if (args[1].equals("start"))
    {
      if (calibrating)
      {
        ULOG_WARNING("Calibration already started");
        return;
      }

      if (args[2].equals("u"))
      {
        calibrating = 1;
        ULOG_INFO("Voltage calibration started");
      }
      else if (args[2].equals("i"))
      {
        calibrating = 2;
        ULOG_INFO("Current calibration started");
      }
      else
      {
        ULOG_WARNING("Invalid argument: %s", args[2].c_str());
      }
      return;
    }

    // cal save
    if (args[1].equals("save"))
    {
      if (!calibrating)
      {
        ULOG_WARNING("Not in calibration mode");
        return;
      }

      calibrating = 0;
      settings.header = 0x69;
      settings.checksum = Tools::calcSum(&settings, sizeof(settings) - 1);
      EEPROM.put(0, settings);
      EEPROM.commit();
      ULOG_INFO("Calibration data saved");
      return;
    }

    // cal exit
    if (args[1].equals("exit"))
    {
      if (!calibrating)
      {
        ULOG_WARNING("Not in calibration mode");
        return;
      }
      calibrating = 0;
      ULOG_INFO("Calibration canceled");
      return;
    }

    // cal scale
    if (args[1].equals("scale"))
    {
      switch (calibrating)
      {
      case 1: // U
      {
        if (args.size() == 3)
        {
          auto scale = args[2].toInt();
          if (scale < 0 || scale > 3)
          {
            ULOG_WARNING("Invalid scale level");
            return;
          }
          uMeter.selectScale(scale);
        }
        auto activeScale = uMeter.getActiveScale();
        ULOG_INFO("Voltage scale: %d, range: %.2fV - %.2fV",
                  activeScale, U_SCALE_MIN_VALUE[activeScale], U_SCALE_MAX_VALUE[activeScale]);
        return;
      }
      case 2: // I
      {
        if (args.size() == 3)
        {
          auto scale = args[2].toInt();
          if (scale < 0 || scale > 3)
          {
            ULOG_WARNING("Invalid scale level");
            return;
          }
          uMeter.selectScale(scale);
        }
        auto activeScale = iMeter.getActiveScale();
        ULOG_INFO("Current scale: %d, range: %.2fA - %.2fA",
                  activeScale, I_SCALE_MIN_VALUE[activeScale], I_SCALE_MAX_VALUE[activeScale]);
        return;
      }
      default:
        ULOG_WARNING("Not in calibration mode");
        return;
      }
    }

    // cal in
    if (args[1].equals("in"))
    {
      auto inputValue = args[2].toFloat();

      switch (calibrating)
      {
      case 1: // U
      {
        auto activeScale = uMeter.getActiveScale();
        if (inputValue < U_SCALE_MIN_VALUE[activeScale] || inputValue > U_SCALE_MAX_VALUE[activeScale])
        {
          ULOG_WARNING("Input value out of range");
          return;
        }

        auto rawV = uMeter.getRawVoltage();
        auto gain = rawV / inputValue;
        settings.vScaleGains[activeScale] = gain;
        ULOG_INFO("Voltage scale %d gain: %.4f", activeScale, gain);
        return;
      }

      case 2: // I
      {
        auto activeScale = iMeter.getActiveScale();
        if (inputValue < I_SCALE_MIN_VALUE[activeScale] || inputValue > I_SCALE_MAX_VALUE[activeScale])
        {
          ULOG_WARNING("Input value out of range");
          return;
        }

        auto rawV = iMeter.getRawVoltage();
        auto gain = rawV / (inputValue * I_SAMPLE_RES);
        settings.iScaleGains[activeScale] = gain;
        ULOG_INFO("Current scale %d gain: %.4f", activeScale, gain);
        return;
      }

      default:
        ULOG_WARNING("Not in calibration mode");
        return;
      }
    }
  };

  Console::Command calCmd{"cal", help_cal, 1, 2, cmdCalCallback};
  Console::registerCommand(calCmd);

  while (1)
  {
    auto time0 = millis();

    if (!(millis() % SAMPLE_PERIOD))
    {
      uMeter.convertOnce();
      iMeter.convertOnce();
    }

    if (!(millis() % GET_VALUE_PERIOD))
    {
      auto uValue = uMeter.readVoltage();
      auto iValue = iMeter.readVoltage();
      iValue /= I_SAMPLE_RES;

      // Scale auto-adjustment
      if (uValue >= 0 && !(calibrating == 1)) // Voltage is valid and not in calibration mode
      {
        auto activeScale = uMeter.getActiveScale();
        if (uValue > U_SCALE_MAX_VALUE[activeScale]) // Too high
        {
          if (activeScale > 0)
          {
            uMeter.selectScale(activeScale - 1);
            uValue = -1; // Invalidate the value
          }
          else
          {
            uValue = INFINITY; // Overload
          }
        }
        else if (uValue < U_SCALE_MIN_VALUE[activeScale]) // Too low
        {
          if (activeScale < 3)
          {
            uMeter.selectScale(activeScale + 1);
          }
        }
      }

      if (iValue >= 0 && !(calibrating == 2)) // Current is valid and not in calibration mode
      {
        auto activeScale = iMeter.getActiveScale();
        if (iValue > I_SCALE_MAX_VALUE[activeScale]) // Too high
        {
          if (activeScale > 0)
          {
            iMeter.selectScale(activeScale - 1);
            iValue = -1; // Invalidate the value
          }
          else
          {
            iValue = INFINITY; // Overload
          }
        }
        else if (iValue < I_SCALE_MIN_VALUE[activeScale]) // Too low
        {
          if (activeScale < 3)
          {
            iMeter.selectScale(activeScale + 1);
          }
        }
      }

      ULOG_DEBUG("Voltage: %f V (%d), Current: %f A (%d)", uValue, uMeter.getActiveScale(), iValue, iMeter.getActiveScale());
      Display::updateVoltage(uValue);
      Display::updateCurrent(iValue);
    }

    if (!(millis() % CONSOLE_HANDLE_PERIOD))
    {
      Console::handleConsoleEvent();
    }

    while (millis() == time0)
      ;
  }
}

// Useless
void loop()
{
}

// For UI driver
void setup1()
{
  KeyPad keyPad;
  keyPad.addKey(KEY_R_PIN);
  keyPad.addKey(KEY_L_PIN);
  keyPad.addKey(KEY_OK_PIN);

  auto readKey = [&keyPad] -> std::pair<uint32_t, bool>
  {
    auto [k, s] = keyPad.getLastKeyEvent();
    uint32_t keyID;
    switch (k)
    {
    case KEY_R_PIN:
      keyID = Display::LV_KEY_NEXT;
      break;
    case KEY_L_PIN:
      keyID = Display::LV_KEY_PREV;
      break;
    case KEY_OK_PIN:
      keyID = Display::LV_KEY_ENTER;
      break;
    default:
      keyID = 0;
    }

    return {keyID, s};
  };

  Display::init();
  Display::setReadKeyEventCb(readKey);

  while (true)
  {
    Display::run();
  }
}
