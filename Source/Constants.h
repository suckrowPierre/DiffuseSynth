#pragma once

#include <juce_core/juce_core.h>

namespace AudioPluginConstants
{
    const juce::String initialPromptFieldMessage = "Prompt me";
    const juce::String initialNegativePromptFieldMessage = "low quality, average quality, noise, high pitch, artefacts";
    const float initialAudioLength = 5.12f;
    const int initialNumInference = 10;
    const float initialGuidanceScale = 2.5f;
    const juce::String red = "FFFF0000";
    const juce::String green = "FF06FF00";
    const juce::String GUIServerStatusId = "server_status";
    const juce::String GUIModelStatusId = "model_status";
    const juce::StringArray devices = { "cpu", "cuda", "mps" };
    const juce::StringArray models = {"audioldm-s-full", "audioldm-s-full-v2", "audioldm-m-full", "audioldm-l-full"};
    const bool initialAutoStartServer = false;
    const bool initialAutoModelSetup = false;

}
