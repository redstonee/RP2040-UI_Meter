#include <ulog.h>
#include <vector>

#include "Console.h"
#include "config.h"

extern "C"
{
    extern const char help_help[];
}

namespace Console
{
    static char buffer[CONSOLE_BUFFER_SIZE];
    static auto bufferPos = 0;
    static std::vector<Command> commands;

    /**
     * @brief Callback for the help command
     *
     * @param args
     */
    static void cmdHelpCallback(std::span<String> args)
    {
        if (args.size() == 1)
        {
            for (auto &cmd : commands)
            {
                Serial.printf("%s - %s\n", cmd.name, cmd.help);
            }
        }
        else
        {
            bool found = false;
            for (auto &cmd : commands)
            {
                if (cmd.name == args[1])
                {
                    found = true;
                    Serial.printf("%s - %s\n", cmd.name, cmd.help);
                    break;
                }
            }
            if (!found)
                ULOG_WARNING("Unknown command: %s", args[1].c_str());
        }
    }

    void init()
    {
        Serial.setTimeout(20);
        Serial.begin(115200);
        static auto logOutput = [](ulog_level_t level, char *msg)
        {
            Serial.printf("[%d] ", millis());
            Serial.print(ulog_level_name(level));
            Serial.print(": ");
            Serial.println(msg);
        };
        ulog_subscribe(logOutput, LOG_LEVEL);

        Command helpCmd{"help", help_help, 0, 1, cmdHelpCallback};
        registerCommand(helpCmd);
    }

    void registerCommand(const Command &cmd)
    {
        if (cmd.maxArgCount < cmd.minArgCount)
        {
            ULOG_ERROR("Unable to register command: %s, minArgCount is greater than maxArgCount", cmd.name);
            return;
        }

        for (auto &c : commands)
        {
            if (cmd.name == c.name)
            {
                ULOG_ERROR("Unable to register command: %s, command already exists", cmd.name);
                return;
            }
        }

        if (!cmd.cb)
        {
            ULOG_ERROR("Unable to register command: %s, callback is empty", cmd.name);
            return;
        }

        commands.push_back(cmd);
    }

    /**
     * @brief Parse the command line arguments from the buffer
     *
     * @return std::vector<String> The parsed arguments, first element is the command
     */
    static std::vector<String> parseArgs()
    {
        char str[CONSOLE_BUFFER_SIZE];
        strncpy(str, buffer, bufferPos + 1);
        str[bufferPos] = '\0';

        std::vector<String> args;
        char *token = strtok(str, " ");
        while (token)
        {
            args.push_back(token);
            token = strtok(NULL, " ");
        }
        return args;
    }

    void handleConsoleEvent()
    {
        while (Serial.available())
        {
            char c = Serial.read();
            if (bufferPos >= CONSOLE_BUFFER_SIZE)
            {
                bufferPos = 0;
                ULOG_WARNING("Console buffer overflow");
            }
            else if (c == '\b') // Backspace
            {
                if (bufferPos > 0)
                {
                    Serial.print("\b \b");
                    bufferPos--;
                }
            }
            else
            {
                Serial.print(c); // Echo
                if (c == '\r')   // Carriage return
                    continue;

                if (c == '\n') // Enter
                {
                    if (bufferPos == 0) // Nothing to process
                    {
                        Serial.print(CONSOLE_PROMPT);
                        continue;
                    }

                    auto args = parseArgs();
                    bool found = false;
                    for (auto &cmd : commands) // Look for the command
                    {
                        if (args[0] == cmd.name)
                        {
                            found = true;
                            if (args.size() - 1 < cmd.minArgCount || args.size() - 1 > cmd.maxArgCount)
                                ULOG_WARNING("Invalid argument count for command: %s", cmd.name);
                            else
                                cmd.cb(args);

                            break;
                        }
                    }
                    if (!found)
                        ULOG_WARNING("Unknown command: %s, try 'help'", args[0].c_str());

                    bufferPos = 0;
                    Serial.print(CONSOLE_PROMPT);
                    break;
                }
                else
                {
                    buffer[bufferPos++] = c;
                }
            }
        }
    }
} // namespace Console
