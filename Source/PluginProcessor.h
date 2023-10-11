#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <foleys_gui_magic/foleys_gui_magic.h>
#include "Util/Constants.h"
#include "Util/GuiHandler.h"
#include "Api//ApiHandler.h"
#include "Components/SampleHolder.h"
#include "Components/MySampler.h"


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
    int getPort() const;

    std::map<juce::String, int> ParameterIndexMap;
private:

    void loadFile();
    void setupProcessor();
    void setReferenceValues();
    void registerEventTriggers();
    void generateSampleFromPrompt();
    void refresh() const;
    void startServer();
    void initModel() const;
    juce::ADSR::Parameters ADSRParameters;

    juce::AudioParameterFloat* pitch  = nullptr;
    juce::AudioParameterFloat* attack  = nullptr;
    juce::AudioParameterFloat* decay   = nullptr;
    juce::AudioParameterFloat* sustain = nullptr;
    juce::AudioParameterFloat* release = nullptr;
    juce::AudioParameterFloat* gain    = nullptr;


    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    juce::AudioProcessorValueTreeState apvts;
    juce::Synthesiser sampler;
    juce::AudioFormatManager formatManager;
    juce::AudioFormatReader* reader = nullptr;
    juce::AudioBuffer<float> waveForm;
    foleys::SampleHolder* audioThumbnail = nullptr;
    juce::AudioThumbnailCache thumbnailCache{64};
    foleys::MagicLevelSource* outputMeter = nullptr;
    juce::Value promptValue{AudioPluginConstants::initialPromptFieldMessage};
    juce::Value negativePromptValue{AudioPluginConstants::initialNegativePromptFieldMessage};
    juce::Value seedValue{""};
    static void addFloatParameter(std::vector<std::unique_ptr<juce::RangedAudioParameter>>& parameters,
                                  const std::string& id, const std::string& name,
                                  float min, float max, float defaultVal);
    juce::RangedAudioParameter* getParameter(const std::string& paramId);
    void updateADSRParameters();
    void fetchADSRParameters();
    void updatePitch();

    static void addChoiceParameters(std::vector<std::unique_ptr<juce::RangedAudioParameter>>& parameters);
    static void addIntParameters(std::vector<std::unique_ptr<juce::RangedAudioParameter>>& parameters);
    static void addBoolParameters(std::vector<std::unique_ptr<juce::RangedAudioParameter>>& parameters);
    static void addFloatParameters(std::vector<std::unique_ptr<juce::RangedAudioParameter>>& parameters);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessor)

    void addParamListeners();

    juce::String validateSeed(const juce::String &seedString);

    void logAndShowException(const std::exception &e) const;
};
