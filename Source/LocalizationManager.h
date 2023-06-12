#pragma once
//
// Created by Pierre-Louis Suckrow on 12.06.23.
//
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_core/juce_core.h>


#ifndef DIFFUSESYNTH_LOCALIZATIONMANAGER_H
#define DIFFUSESYNTH_LOCALIZATIONMANAGER_H

enum class Language {
    English,
    German
};


class LocalizationManager {
public:
    static LocalizationManager& getInstance();

    void setLanguage(const Language& language);
    Language getLanguage();

    juce::String getLocalizedString(const juce::String& key);

private:
    explicit LocalizationManager(const Language& initialLanguage = Language::English);
    ~LocalizationManager();

    LocalizationManager(const LocalizationManager&) = delete;
    LocalizationManager& operator=(const LocalizationManager&) = delete;

    Language language;
    std::unordered_map<juce::String, juce::String> loadLanguageFile();
    juce::String currentLanguageFile;
    std::unordered_map<juce::String, juce::String> localizedStrings;
};


#endif //DIFFUSESYNTH_LOCALIZATIONMANAGER_H
