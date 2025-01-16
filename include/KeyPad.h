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

public:
    KeyPad(std::span<const int> keys, bool activeLow = true);
    KeyPad(bool activeLow = true);

    friend void onKeyStateChange(void *params);

    void addKey(int key);

    std::pair<int, bool> getLastKeyEvent();
};