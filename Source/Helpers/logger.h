#pragma once

#include <string>
#include <juce_audio_utils/juce_audio_utils.h>z

class Logger
{
public:

    static void log(
        const std::string& message
    );

    static void clear();

    static std::string getLogPath();
};