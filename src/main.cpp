#include <Arduino.h>
#include <EEPROM.h>
#include <ulog.h>

#include "Tools.hpp"
#include "UpLink.h"
#include "Display.h"
#include "KeyPad.h"
#include "config.h"
#include "VoltMeter.hpp"

struct MeterSettings
{
  float vScaleGains[4];
  float iScaleGains[4];

  uint8_t checksum; // XOR of the payload bytes
};

/**
 * @brief The entry point of the program
 */
void setup()
{
  UpLink::init();
  analogReadResolution(ADC_RESOLUTION);

  VoltMeter uMeter(USENSE_PIN, U_SCALE0_PIN, U_SCALE1_PIN);
  VoltMeter iMeter(ISENSE_PIN, I_SCALE0_PIN, I_SCALE1_PIN);

  MeterSettings settings;
  EEPROM.begin(sizeof(settings));
  EEPROM.get(0, settings);

  auto sum = Tools::calcSum(reinterpret_cast<uint8_t *>(&settings), sizeof(settings) - 1);
  if (sum != settings.checksum)
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

  uMeter.selectScale(2);
  iMeter.selectScale(2);

  while (1)
  {
    if (!(millis() % SAMPLE_PERIOD))
    {
      uMeter.convertOnce();
      iMeter.convertOnce();
    }

    if (!(millis() % GET_VALUE_PERIOD))
    {
      auto [uValid, uValue] = uMeter.readVoltage();
      auto [iValid, iValue] = iMeter.readVoltage();
      iValue *= I_SAMPLE_RES;

      // Current meter scale adjustment
      auto activeScale = uMeter.getActiveScale();
      if (uValid && uValue > U_SCALE_MAX_VALUE[activeScale])
      {
        if (activeScale < 3)
        {
          uMeter.selectScale(activeScale + 1);
          uValid = false;
        }
        else
        {
          uValue = INFINITY;
        }
      }
      else if (uValid && uValue < U_SCALE_MIN_VALUE[activeScale])
      {
        if (activeScale > 0)
        {
          uMeter.selectScale(activeScale - 1);
          uValid = false;
        }
      }

      // Voltage meter scale adjustment
      activeScale = iMeter.getActiveScale();
      if (iValid && iValue > I_SCALE_MAX_VALUE[activeScale])
      {
        if (activeScale < 3)
        {
          iMeter.selectScale(activeScale + 1);
          iValid = false;
        }
        else
        {
          iValue = INFINITY;
        }
      }
      else if (iValid && iValue < I_SCALE_MIN_VALUE[activeScale])
      {
        if (activeScale > 0)
        {
          iMeter.selectScale(activeScale - 1);
          iValid = false;
        }
      }

      ULOG_DEBUG("Voltage: %fV, Current: %fA", uValue, iValue);
      sendValue(uValue, iValue);
      Display::updateVoltage(uValid, uValue);
      Display::updateCurrent(iValid, iValue);
    }
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
