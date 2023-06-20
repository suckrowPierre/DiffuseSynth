//
// Created by Pierre-Louis Suckrow on 12.06.23.
//

#include "LocalizationManager.h"
#include "../lib/magic_enum.hpp"
#include "BinaryData.h" //Gets generated in the build process

LocalizationManager& LocalizationManager::getInstance() {
    static LocalizationManager instance;
    return instance;
}

LocalizationManager::LocalizationManager(const Language& initialLanguage) {
    setLanguage(initialLanguage);
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
    juce::String resourceName = juce::String(languageName).toLowerCase() + "_json";

    const char* resourceData = nullptr;
    int resourceSize = 0;

    if (auto* resource = BinaryData::getNamedResource(resourceName.toRawUTF8(), resourceSize)) {
        resourceData = resource;
    }

    juce::String fileContent = juce::String::fromUTF8(resourceData, resourceSize);

    auto jsonVar = juce::JSON::parse(fileContent);

    std::unordered_map<juce::String, juce::String> tempMap;
    if (jsonVar.isObject()) {
        flattenJSONObject(jsonVar.getDynamicObject(), tempMap, "");
    }

    return tempMap;
}

void LocalizationManager::flattenJSONObject(juce::DynamicObject* jsonObject, std::unordered_map<juce::String, juce::String>& map, juce::String currentKey) {
    auto properties = jsonObject->getProperties();
    for (auto& property : properties) {
        juce::String newKey = currentKey.isEmpty() ? property.name.toString() : currentKey + "." + property.name.toString();
        if (property.value.isObject()) {
            flattenJSONObject(property.value.getDynamicObject(), map, newKey);
        } else {
            map[newKey] = property.value.toString();
        }
    }
}




