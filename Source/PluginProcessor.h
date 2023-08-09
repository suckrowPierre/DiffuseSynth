#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "../Modules/foleys_gui_magic/foleys_gui_magic.h"
#include "Constants.h"
#include "GuiHandler.h"
#include "Api//ApiHandler.h"
#include "Components/SampleHolder.h"
#include "Waveform.h"

//==============================================================================
class AudioPluginAudioProcessor  : public foleys::MagicProcessor
{
public:
    AudioPluginAudioProcessor();
    ~AudioPluginAudioProcessor() override;


    std::unique_ptr<ApiHandler> apiHandler;
    std::unique_ptr<GuiHandler> guiHandler;

    //==============================================================================
    // AudioProcessor overrides
    //==============================================================================
    void initialiseBuilder (foleys::MagicGUIBuilder& builder) override;
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;
    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    // Program management
    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;


    //==============================================================================
    // Program getters
    //==============================================================================
    juce::String getModelProperty() const;
    juce::String getDeviceProperty() const;
    bool isAutoStartServer();
    bool isAutoModelSetup();
    int getPort() const;

private:

    void loadFile();
    void setupProcessor();
    void setReferenceValues();
    void registerEventTriggers();
    void generateSampleFromPrompt();
    void refresh() const;
    void startServer();
    void initModel() const;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    juce::AudioProcessorValueTreeState apvts;
    juce::Synthesiser sampler;
    juce::AudioFormatManager formatManager;
    juce::AudioFormatReader* reader = nullptr;
    juce::AudioBuffer<float> waveForm;
    foleys::WaveformHolder* audioThumbnail = nullptr;
    juce::AudioThumbnailCache thumbnailCache{64};
    foleys::MagicLevelSource* outputMeter = nullptr;
    juce::Value promptValue{AudioPluginConstants::initialPromptFieldMessage};
    juce::Value negativePromptValue{AudioPluginConstants::initialNegativePromptFieldMessage};


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessor)
};
