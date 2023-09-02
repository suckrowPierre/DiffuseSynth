#pragma once

#include "juce_core/juce_core.h"

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
    const juce::StringArray models = {"audioldm-s-full-v2", "audioldm-m-full", "audioldm-l-full"};
    const bool initialAutoStartServer = false;
    const bool initialAutoModelSetup = false;
    const int numVoices = 1;
    const juce::String tempFileName = "temp.wav";
    const float maxPitch = 12.0f;
    const float minPitch = -12.0f;
    const int maxKeepMS = 300;

    const float minAttack = 0.0f;
    const float maxAttack = 10.0f;
    const float defaultAttack = 0.0f;
    const float minDecay = 0.0f;
    const float maxDecay = 10.0f;
    const float defaultDecay = 0.0f;
    const float minSustain = 0.0f;
    const float maxSustain = 1.0f;
    const float defaultSustain = 1.0f;
    const float minRelease = 0.0f;
    const float maxRelease = 10.0f;
    const float defaultRelease = 0.0f;
    const float minGain = 0.0f;
    const float maxGain = 1.5f;

}
