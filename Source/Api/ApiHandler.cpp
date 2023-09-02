//
// Created by Pierre-Louis Suckrow on 12.07.23.
//
#include "../util/Logger.h"
#include "ApiHandler.h"
#include "../PluginProcessor.h"
#include <filesystem>
#include <iostream>

ApiHandler::ApiHandler(AudioPluginAudioProcessor &processor)
        : processor(processor), apiClient(std::make_unique<AudioLDMApiClient>()) {
}

bool ApiHandler::isApiClientInitialized() const {
    if (!apiClient || apiClient->getApiPort() == "") {
        return false;
    }
    return true;
}

void ApiHandler::checkApiClientInitialized() const {
    if (!apiClient) {
        throw std::runtime_error("AudioLDMApiClient: not initialized");
    }
    if (apiClient->getApiPort() == "") {
        throw std::runtime_error("AudioLDMApiClient: port not set");
    }
}

void ApiHandler::initializeApiConnection(bool autoStartServer, bool autoSetupModel) {
    apiClient->setApiPort(juce::String(processor.getPort()));
    checkApiClientInitialized();
    Logger::logInfo("Initializing API connection");
    if (checkApiStatus()) {
        if (!checkModelStatus()) {
            if (processor.isAutoModelSetup()) {
                autoInitModel();
            }
        } else {
            juce::Logger::writeToLog("Model setup");
        }
    }
    fetchStatusAndParameters();
}

void ApiHandler::startServer(int &port) {
    juce::File currentFile(juce::File::getSpecialLocation(juce::File::currentExecutableFile));
    juce::File binaryDir = currentFile.getParentDirectory();
    std::cout << "Binary directory: " << binaryDir.getFullPathName() << std::endl;
}

void ApiHandler::initModel(juce::String device, juce::String repo_id) {
    checkApiClientInitialized();
    SetupModelParameters params{device, repo_id};

    juce::Logger::writeToLog("Setting up model with parameters: " + params.device + ", " + params.repo_id);
    try {
        apiClient->setupModel(params);
        juce::Logger::writeToLog("successfully set up model");
    } catch (const std::exception &e) {
        throw std::runtime_error("Failed to set up model. Exception: " + std::string(e.what()));
    }
    fetchStatusAndParameters();
}

bool ApiHandler::checkModelStatus() {
    if (!isApiClientInitialized()) {
        return false;
    }
    bool isAvailable = false;
    try {
        isAvailable = apiClient->isModelSetUp();
    } catch (const std::exception &e) {
        Logger::logException(e);
    }

    const auto statusMessage = isAvailable ? "Model is available" : "Model is not available";
    const auto statusColour = isAvailable ? AudioPluginConstants::green : AudioPluginConstants::red;

    Logger::logInfo(statusMessage);
    processor.guiHandler->updateGUIStatus(AudioPluginConstants::GUIModelStatusId, statusColour);

    return isAvailable;

}

bool ApiHandler::checkApiStatus() {
    if (!isApiClientInitialized()) {
        return false;
    }
    bool isAvailable = false;
    try {
        isAvailable = apiClient->isApiAvailable();
    } catch (const std::exception &e) {
        Logger::logException(e);
    }
    const auto statusMessage = isAvailable ? "API is available" : "API is not available";
    const auto statusColour = isAvailable ? AudioPluginConstants::green : AudioPluginConstants::red;

    Logger::logInfo(statusMessage);
    processor.guiHandler->updateGUIStatus(AudioPluginConstants::GUIServerStatusId, statusColour);

    return isAvailable;
}

void ApiHandler::generateSample(juce::String prompt, juce::String negative_prompt, float audio_length_in_s,
                                int num_inference_steps, float guidance_scale, juce::String seed) {
    checkApiClientInitialized();

    GenerateSampleParameters params{prompt, negative_prompt, audio_length_in_s, num_inference_steps, guidance_scale,
                                    seed};
    juce::Logger::writeToLog(
            "Generating sample with parameters: " + params.prompt + ", " + params.negative_prompt + ", " +
            juce::String(params.audio_length_in_s) + ", " + juce::String(params.num_inference_steps) + ", " +
            juce::String(params.guidance_scale) + ", " + juce::String(params.seed));

    try {
        apiClient->generateSample(params);
        juce::Logger::writeToLog("Sample generated");
    } catch (const std::exception &e) {
        throw std::runtime_error("Failed to generate sample. Exception: " + std::string(e.what()));
    }

}

void ApiHandler::fetchStatusAndParameters() {
    apiClient->setApiPort(juce::String(processor.getPort()));
    checkApiStatus();
    checkModelStatus();
}
