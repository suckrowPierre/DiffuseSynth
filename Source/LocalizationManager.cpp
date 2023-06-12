//
// Created by Pierre-Louis Suckrow on 12.06.23.
//

#include "LocalizationManager.h"
#include "magic_enum.hpp"

LocalizationManager::LocalizationManager(const Language& initialLanguage) {
    setLanguage(initialLanguage);
}
LocalizationManager::LocalizationManager() {
    setLanguage(Language::English);
}
LocalizationManager::~LocalizationManager() {
}

Language LocalizationManager::getLanguage() {
    return language;
}

void LocalizationManager::setLanguage(const Language& languageNew) {
    this->language = languageNew;
    this->localizedStrings = loadLanguageFile();
}

juce::String LocalizationManager::getLocalizedString(const juce::String &key) {
    auto it = localizedStrings.find(key);
    if (it != localizedStrings.end()) {
        return it->second;
    }
    std::cerr << "ERROR: Key not found: " << key << std::endl;
    return "missing Translation";
}

std::unordered_map<juce::String, juce::String> LocalizationManager::loadLanguageFile() {
    std::string languageName = std::string(magic_enum::enum_name(language));
    juce::String fileName = juce::String(languageName).toLowerCase() + ".json";
    juce::File languageFile = juce::File::getSpecialLocation(juce::File::currentApplicationFile)
            .getChildFile ("Resources")
            .getChildFile ("lng")
            .getChildFile (fileName);

    juce::String fileContent = languageFile.loadFileAsString();

    auto jsonVar = juce::JSON::parse(fileContent);

    std::unordered_map<juce::String, juce::String> tempMap;
    if (jsonVar.isObject()) {
        auto* json = jsonVar.getDynamicObject();
        auto properties = json->getProperties();
        for (auto& property : properties) {
            tempMap[property.name.toString()] = property.value.toString();
        }
    }

    return tempMap;
}



