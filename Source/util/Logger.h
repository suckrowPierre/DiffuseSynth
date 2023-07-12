//
// Created by Pierre-Louis Suckrow on 12.07.23.
//

#ifndef DIFFUSESYNTH_LOGGER_H
#define DIFFUSESYNTH_LOGGER_H

#include "juce_audio_processors/juce_audio_processors.h"


class Logger {
public:
    Logger();
    ~Logger();

    static void logError(const juce::String& message);
    static void logInfo(const juce::String& message);
    static void logException(const std::exception& e);

};


#endif //DIFFUSESYNTH_LOGGER_H
