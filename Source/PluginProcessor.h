#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "../Modules/foleys_gui_magic/foleys_gui_magic.h"
#include "AudioLDMApiClient.h"

//==============================================================================
class AudioPluginAudioProcessor  : public foleys::MagicProcessor
{
public:
    //==============================================================================
    AudioPluginAudioProcessor();
    ~AudioPluginAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;


    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================

private:
    //==============================================================================
    void connectToApi();
    void generateSampleFromPrompt();
    void setupModel(juce::String device, juce::String repo_id);
    std::unique_ptr<AudioLDMApiClient> apiClient;
    bool checkApiStatus();
    bool checkModelStatus();
    void refresh();

    std::map<juce::String, std::vector<int>> nodePaths;
    juce::ValueTree getNodeById(const juce::String& id);
    juce::ValueTree getNodeByPath(const std::vector<int>& path);
    std::vector<int> searchGUITreeRecursively(const juce::ValueTree& node, const juce::Identifier& identifier, const juce::String& id, std::vector<int> path = {});


    void updateGUIStatus(const juce::String& id, const juce::String& colour);
    void setListBoxes(juce::Array<juce::var>& items);


    juce::String initialPromptFieldMessage = "Prompt Me";
    juce::String initialNegativePromptFieldMessage = "low quality, average quality, noise, high pitch, artefacts";
    juce::Value promptValue { initialPromptFieldMessage};
    juce::Value negativePromptValue {initialNegativePromptFieldMessage};

    float initialAudioLength = 5.12f;
    juce::Value audioLengthValue { initialAudioLength };
    float initialNumInference = 10.0f;
    juce::Value numInferenceValue { initialNumInference };
    float initialGuidanceScale = 2.5f;
    juce::Value guidanceScaleValue { initialGuidanceScale };

    juce::String red = "FFFF0000";
    juce::String green = "FF06FF00";
    juce::String GUIServerStatusId = "server_status";
    juce::String GUIModelStatusId = "model_status";



    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessor)

    static void extractDeviceAndModelParameters(const juce::var &devicesAndModels) ;
};
