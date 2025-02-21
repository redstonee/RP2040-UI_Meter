const char help_help[] = "Display the help message\n"
                         "  Usage: help [command]\n";

const char help_cal[] = "Calibrate the current and voltage scales\n"
                        "  Usage: cal <start|save|exit|scale|in|gains> [options]\n"
                        "\tcal start <u|i> - Start the calibration process for the voltage or current scale, "
                        "should be run before other calibration commands.\n"
                        "\tcal save - Save the calibration data for the voltage or current scale and exit\n"
                        "\tcal exit - Exit the calibration process and discard the changes\n"
                        "\tcal scale [level] - Show or set the scale level(0-3)\n"
                        "\tcal in <value> - Input the actual value(in V or A)\n"
                        "\tcal gains - Show the current gains setting\n";
