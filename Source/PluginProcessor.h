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
    void generateSampleFromPrompt(const juce::String& prompt, const juce::String& negative_prompt); // Added const
    void setupModel(juce::String device, juce::String repo_id);
    std::unique_ptr<AudioLDMApiClient> apiClient;

    juce::Label editableLabel;
    juce::Label negativePromptLabel;
    juce::String initialPromptFieldMessage = "Prompt Me";
    juce::String initialNegativePromptFieldMessage = "low quality, average quality, noise, high pitch, artefacts";
    juce::Value promptValue { initialPromptFieldMessage};
    juce::Value negativePromptValue {initialNegativePromptFieldMessage};


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessor)
};
