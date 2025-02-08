#pragma once

#include <vector>
#include <span>
#include <utility>

class KeyPad
{
private:
    std::vector<int> keys;
    bool activeState;

    int lastKeyPin;
    bool lastKeyState;

    struct KeyEvent
    {
        KeyPad *keypad;
        int key;
    };

    static void onKeyStateChange(void *params)
    {
        auto event = static_cast<KeyEvent *>(params);
        event->keypad->lastKeyPin = event->key;
        event->keypad->lastKeyState = (digitalRead(event->key) == event->keypad->activeState);
    }

public:
    KeyPad(std::span<const int> keys, bool activeLow = true) : activeState(!activeLow)
    {
        for (auto p : keys)
        {
            addKey(p);
        }
    }

    KeyPad(bool activeLow = true) : activeState(!activeLow)
    {
    }

    void addKey(int key)
    {
        keys.push_back(key);
        pinMode(key, INPUT_PULLUP);
        auto params = new KeyEvent{this, key};
        attachInterruptParam(key, onKeyStateChange, CHANGE, static_cast<void *>(params));
    }

    std::pair<int, bool> getLastKeyEvent()
    {
        return {lastKeyPin, lastKeyState};
    }
};