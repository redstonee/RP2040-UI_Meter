#include <Arduino.h>

#include "Display.h"
#include "KeyPad.h"
#include "config.h"

void setup()
{
}

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

  auto readKey = [&keyPad](uint32_t *keyID, bool *pressed)
  {
    auto [k, s] = keyPad.getLastKeyEvent();
    switch (k)
    {
    case KEY_R_PIN:
      *keyID = Display::LV_KEY_NEXT;
      break;
    case KEY_L_PIN:
      *keyID = Display::LV_KEY_PREV;
      break;
    case KEY_OK_PIN:
      *keyID = Display::LV_KEY_ENTER;
      break;
    default:
      *keyID = 0;
    }
    *pressed = s;
  };

  Display::init();
  Display::setReadKeyEventCb(readKey);
  while (true)
  {
    while (millis() % 10)
      ;
    Display::handleTimer();
  }
}
