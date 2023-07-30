//
// Created by Pierre-Louis Suckrow on 12.07.23.
//

#include "Logger.h"

Logger::Logger() {
}
Logger::~Logger() {
}

void Logger::logError(const juce::String& message)
{
    juce::Logger::writeToLog("Failed to connect to API. " + message);
}

void Logger::logInfo(const juce::String& message)
{
    juce::Logger::writeToLog(message);
}

void Logger::logException(const std::exception& e)
{
    juce::Logger::writeToLog("Exception: " + juce::String(e.what()));
}