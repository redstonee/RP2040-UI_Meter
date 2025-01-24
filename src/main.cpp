#include <Arduino.h>
#include <EEPROM.h>
#include <ulog.h>

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
 * @brief Calculate the sum of an byte (uint8_t) array by XOR
 *
 * @param data A pointer to the byte array
 * @param len The length of the array
 * @return uint8_t sum
 */
uint8_t calcSum(uint8_t *data, size_t len)
{
  uint8_t sum = 0;
  for (size_t i = 0; i < len; i++)
  {
    sum ^= *(data++);
  }
  return sum;
}

void setup()
{
  Serial.begin(115200);
  static auto logOutput = [](ulog_level_t level, char *msg)
  {
    Serial.printf("[%d] ", millis());
    Serial.print(ulog_level_name(level));
    Serial.print(": ");
    Serial.println(msg);
  };
  ulog_subscribe(logOutput, ULOG_INFO_LEVEL);

  analogReadResolution(12);

  VoltMeter uMeter(VSENSE_PIN, V_SCALE0_PIN, V_SCALE1_PIN);
  VoltMeter iMeter(ISENSE_PIN, I_SCALE0_PIN, I_SCALE1_PIN);

  MeterSettings settings;
  EEPROM.begin(sizeof(settings));
  EEPROM.get(0, settings);

  auto sum = calcSum(reinterpret_cast<uint8_t *>(&settings), sizeof(settings) - 1);
  if (sum != settings.checksum)
  {
    ULOG_WARNING("No valid gain settings stored.");
    uMeter.setGains(V_SCALE_DEF_GAINS);
    iMeter.setGains(I_SCALE_DEF_GAIN);
  }
  else
  {
    uMeter.setGains(settings.vScaleGains);
    iMeter.setGains(settings.iScaleGains);
  }

  while (1)
  {
    if (!(millis() % SAMPLE_PERIOD))
    {
      uMeter.convertOnce();
      iMeter.convertOnce();
    }

    if (!(millis() % GET_VALUE_PERIOD))
    {
      uMeter.readVoltage();
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
    Display::handleTimer();
  }
}
