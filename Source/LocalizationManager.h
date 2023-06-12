#pragma once
//
// Created by Pierre-Louis Suckrow on 12.06.23.
//
#include <juce_audio_processors/juce_audio_processors.h>


#ifndef DIFFUSESYNTH_LOCALIZATIONMANAGER_H
#define DIFFUSESYNTH_LOCALIZATIONMANAGER_H

enum class Language {
    English,
    German
};


class LocalizationManager {

public:
    explicit LocalizationManager(const Language& initialLanguage);
    ~LocalizationManager();

    void setLanguage(const Language& language);
    Language getLanguage();

    juce::String getLocalizedString(const juce::String& key);

private:
    Language language;

    juce::String loadLanguageFile();
    juce::String currentLanguageFile;
};


#endif //DIFFUSESYNTH_LOCALIZATIONMANAGER_H
