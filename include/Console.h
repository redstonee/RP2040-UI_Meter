#pragma once
#include <functional>
#include <span>
#include <Arduino.h>

namespace Console
{

    /** Command callback type with various count of args, the first arg would be the command name */
    using CmdCb = std::function<void(std::span<String>)>;

    /** Command structure */
    struct Command
    {
        const char *name;
        const char *help;
        uint8_t minArgCount;
        uint8_t maxArgCount;
        CmdCb cb;
    };

    /**
     * @brief Initialize the serial port and the logging system
     */
    void init();

    /**
     * @brief  Register a command
     *
     * @param cmd  The command to register
     */
    void registerCommand(const Command &cmd);

    /**
     * @brief Send the voltage and current values
     *
     * @param uValue The voltage value
     * @param iValue The current value
     */
    void sendValue(const float uValue, const float iValue);

    /**
     * @brief Handle the console event
     *
     * Should be called in the main loop
     */
    void handleConsoleEvent();

} // namespace UpLink
