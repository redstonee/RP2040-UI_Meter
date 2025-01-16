#include <Arduino.h>
#include "KeyPad.h"

struct KeyEvent
{
    KeyPad *keypad;
    int key;
};

void onKeyStateChange(void *params)
{
    auto event = static_cast<KeyEvent *>(params);
    event->keypad->lastKeyPin = event->key;
    event->keypad->lastKeyState = (digitalRead(event->key) == event->keypad->activeState);
}

KeyPad::KeyPad(std::span<const int> keys, bool activeLow) : activeState(!activeLow)
{
    for (auto p : keys)
    {
        addKey(p);
    }
}

KeyPad::KeyPad(bool activeLow) : activeState(!activeLow)
{
}

void KeyPad::addKey(int key)
{
    keys.push_back(key);
    pinMode(key, INPUT_PULLUP);
    auto params = new KeyEvent{this, key};
    attachInterruptParam(key, onKeyStateChange, CHANGE, static_cast<void *>(params));
}

std::pair<int, bool> KeyPad::getLastKeyEvent()
{
    return {lastKeyPin, lastKeyState};
}