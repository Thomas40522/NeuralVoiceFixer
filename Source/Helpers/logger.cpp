#include "logger.h"

#include <fstream>

std::string Logger::getLogPath()
{
    auto logFile =
    juce::File::getSpecialLocation(
        juce::File::userDocumentsDirectory
    )
    .getChildFile(
        "NeuralVoiceFixer.log"
    );

    return logFile
        .getFullPathName()
        .toStdString();

}

void Logger::clear()
{
    std::ofstream log(
        getLogPath(),
        std::ios::trunc
    );
}

void Logger::log(
    const std::string& message
)
{
    std::ofstream log(
        getLogPath(),
        std::ios::app
    );

    if (!log.is_open())
        return;

    log << message << std::endl;
}