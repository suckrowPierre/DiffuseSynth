//
// Created by Pierre-Louis Suckrow on 12.06.23.
//

#include "LocalizationManager.h"
#include "magic_enum.hpp"

LocalizationManager::LocalizationManager(const Language& initialLanguage) {
    setLanguage(initialLanguage);
}
LocalizationManager::~LocalizationManager() {
    setLanguage(Language::English);
}

void LocalizationManager::setLanguage(const Language& languageNew) {
    this->language = languageNew;
    currentLanguageFile = loadLanguageFile();
}

juce::String LocalizationManager::getLocalizedString(const juce::String &key) {
    std:: cout << "key: " << key << std::endl;
    std:: cout << "currentLanguageFile: " << currentLanguageFile << std::endl;
    auto jsonVar = juce::JSON::parse(currentLanguageFile);

    if (jsonVar.isObject())
    {
        auto* json = jsonVar.getDynamicObject();

        if (json->hasProperty(key))
            return json->getProperty(key).toString();
    }

    // Handle error case: key not found or JSON file is not an object
    return "missing Translation"; // Return empty string or some default value
}

juce::String LocalizationManager::loadLanguageFile() {
    std::string languageName = std::string(magic_enum::enum_name(language));

    juce::String fileName = juce::String(languageName).toLowerCase() + ".json";
    juce::File languageFile = juce::File::getSpecialLocation(juce::File::currentApplicationFile)
            .getChildFile ("Resources")
            .getChildFile ("lng")
            .getChildFile (fileName);
    std:: cout << "languageFile: " << languageFile.getFullPathName() << std::endl;
    return languageFile.loadFileAsString();
}



