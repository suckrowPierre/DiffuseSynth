#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "../Modules/foleys_gui_magic/foleys_gui_magic.h"
#include "Constants.h"
#include "GuiHandler.h"
#include "Api//ApiHandler.h"

//==============================================================================
class AudioPluginAudioProcessor  : public foleys::MagicProcessor
{
public:
    AudioPluginAudioProcessor();
    ~AudioPluginAudioProcessor() override;

    juce::AudioProcessorValueTreeState apvts;
    std::unique_ptr<ApiHandler> apiHandler;
    std::unique_ptr<GuiHandler> guiHandler;

    juce::String getModelProperty() const;
    juce::String getDeviceProperty() const;
    bool isAutoStartServer();
    bool isAutoModelSetup();
    int getPort() const;


    void initialiseBuilder (foleys::MagicGUIBuilder& builder) override;

    //==============================================================================
    // AudioProcessor overrides
    //==============================================================================

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

private:

    juce::Synthesiser sampler;
    void loadFile();

    juce::AudioFormatManager formatManager;
    juce::AudioFormatReader* reader = nullptr;

    juce::AudioBuffer<float> waveForm;

    foleys::MagicLevelSource* outputMeter  = nullptr;

    //==============================================================================
    // GUI functions
    //==============================================================================
    void generateSampleFromPrompt();
    void refresh() const;
    void startServer();
    void initModel() const;




    //==============================================================================
    // Other private methods
    //==============================================================================
    void setupProcessor();
    void setReferenceValues();
    void registerEventTriggers();


    //==============================================================================
    // Private member variables
    //==============================================================================
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::Value promptValue { AudioPluginConstants::initialPromptFieldMessage};
    juce::Value negativePromptValue {AudioPluginConstants::initialNegativePromptFieldMessage};



    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessor)
};
