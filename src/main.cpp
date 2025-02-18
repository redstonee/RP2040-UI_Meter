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

    if (args[1].equals("save"))
    {
      if (!calibrating)
      {
        ULOG_WARNING("Not in calibration mode");
        return;
      }

      calibrating = false;
      settings.checksum = Tools::calcSum(&settings, sizeof(settings) - 1);
      EEPROM.put(0, settings);
      EEPROM.commit();
      ULOG_INFO("Calibration data saved");
      return;
    }

    if (args[1].equals("exit"))
    {
      calibrating = false;
      ULOG_INFO("Calibration canceled");
      return;
    }

    if (args[1].equals("scale"))
    {
      if (!calibrating)
      {
        ULOG_WARNING("Not in calibration mode");
        return;
      }

      if (args.size() == 2)
      {
        if (calibrating == 1)
          ULOG_INFO("Voltage scale: %d", uMeter.getActiveScale());
        else if (calibrating == 2)
          ULOG_INFO("Current scale: %d", iMeter.getActiveScale());
      }

      else
      {
        auto scale = args[2].toInt();
        if (scale <= 0 || scale > 3)
        {
          ULOG_WARNING("Invalid scale level");
          return;
        }

        if (calibrating == 1)
        {
          uMeter.selectScale(scale);
          ULOG_INFO("Voltage scale set to %d", scale);
        }
        else if (calibrating == 2)
        {
          iMeter.selectScale(scale);
          ULOG_INFO("Current scale set to %d", scale);
        }
        else
        {
          ULOG_WARNING("Not in calibration mode");
        }
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

      if (uValue >= 0) // Voltage is valid
      {
        // Voltage meter scale adjustment
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

      if (iValue >= 0)
      {
        // Current meter scale adjustment
        auto activeScale = iMeter.getActiveScale();
        if (iValue > I_SCALE_MAX_VALUE[activeScale])
        {
          if (activeScale > 0)
          {
            iMeter.selectScale(activeScale - 1);
            iValue = -1; // Invalidate the value
          }
          else
          {
            iValue = INFINITY;
          }
        }
        else if (iValue < I_SCALE_MIN_VALUE[activeScale])
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
