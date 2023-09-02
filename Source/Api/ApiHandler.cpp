//
// Created by Pierre-Louis Suckrow on 12.07.23.
//
#include "../util/Logger.h"
#include "ApiHandler.h"
#include "../PluginProcessor.h"
#include <filesystem>
#include <iostream>

ApiHandler::ApiHandler(AudioPluginAudioProcessor& processor)
        : processor(processor), apiClient(std::make_unique<AudioLDMApiClient>()) {
}

bool ApiHandler::isApiClientInitialized() const {
    if (!apiClient || apiClient->getApiPort() == "") {
        Logger::logError("AudioLDMApiClient not initialized");
        return false;
    }
    return true;
}

void ApiHandler::initializeApiConnection(bool autoStartServer, bool autoSetupModel)
{
    apiClient->setApiPort(juce::String(processor.getPort()));
    if (isApiClientInitialized()) {
        std::cout << "ApiHandler::initializeApiConnection()" << std::endl;
        try {
            if (checkApiStatus()) {
                juce::Logger::writeToLog("Here");
                if (!checkModelStatus()) {
                    if(processor.isAutoModelSetup()){
                        autoInitModel();
                    }
                } else {
                    juce::Logger::writeToLog("Model setup");
                }
            }
        } catch (const std::exception& e) {
            Logger::logException(e);
        }
        fetchStatusAndParameters();
    }
}

void ApiHandler::autoStartServer() {
    if(isApiClientInitialized()) {
        //TODO
    }
}

void ApiHandler::startServer(int& port)
{
    juce::File currentFile(juce::File::getSpecialLocation(juce::File::currentExecutableFile));
    juce::File binaryDir = currentFile.getParentDirectory();
    std::cout << "Binary directory: " << binaryDir.getFullPathName() << std::endl;
}

void ApiHandler::autoInitModel() {
    if(isApiClientInitialized()) {
        initModel(processor.getDeviceProperty(),processor.getModelProperty());
    }
}

void ApiHandler::initModel(juce::String device, juce::String repo_id) {
    if(!isApiClientInitialized()) {
        return;
    }

    SetupModelParameters params{device, repo_id};



    try {
        if (apiClient->setupModel(params)){
            juce::Logger::writeToLog("successfully set up model");
        }
    } catch (const std::exception& e) {
        juce::Logger::writeToLog("Failed to set up model. Exception: " + juce::String(e.what()));
    }
    fetchStatusAndParameters();
}

bool ApiHandler::checkModelStatus()
{
    if(!isApiClientInitialized()) {
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

bool ApiHandler::checkApiStatus()
{
    if(!isApiClientInitialized()) {
        return false;
    }

    bool isAvailable = apiClient->isApiAvailable();
    const auto statusMessage = isAvailable ? "API is available" : "API is not available";
    const auto statusColour = isAvailable ? AudioPluginConstants::green : AudioPluginConstants::red;

    Logger::logInfo(statusMessage);
    processor.guiHandler->updateGUIStatus(AudioPluginConstants::GUIServerStatusId, statusColour);

    return isAvailable;
}

void ApiHandler::generateSample(juce::String prompt, juce::String negative_prompt, float audio_length_in_s,
                                int num_inference_steps, float guidance_scale, juce::String seed) {
    if(!isApiClientInitialized()) {
        return;
    }

    GenerateSampleParameters params{prompt, negative_prompt, audio_length_in_s, num_inference_steps, guidance_scale, seed};



        juce::Logger::writeToLog("Generating sample with parameters: " + params.prompt + ", " + params.negative_prompt + ", " + juce::String(params.audio_length_in_s) + ", " + juce::String(params.num_inference_steps) + ", " + juce::String(params.guidance_scale) + ", " + juce::String(params.seed));

        try {
            if (apiClient->generateSample(params)){
                juce::Logger::writeToLog("Sample generated");
            }

        } catch (const std::exception& e) {
            juce::Logger::writeToLog("Failed to generate sample. Exception: " + juce::String(e.what()));
        }

}

void ApiHandler::fetchStatusAndParameters()
{
    apiClient->setApiPort(juce::String(processor.getPort()));
    checkApiStatus();
    checkModelStatus();
    //getCurrentParams();
}

void ApiHandler::getCurrentParams()
{
    if(!isApiClientInitialized()) {
        return;
    }

    juce::var currentParams = apiClient->getCurrentParameters();
    juce::var device = currentParams.getProperty("device", "unknown");
    juce::var model = currentParams.getProperty("model", "unknown");

    juce::Logger::writeToLog("Current parameters: " + device.toString() + ", " + model.toString());
}
