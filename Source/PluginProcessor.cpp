#include "PluginProcessor.h"
#include "BinaryData.h"
#include "GuiHandler.h"
#include "api/ApiHandler.h"
#include "util/Logger.h"

juce::AudioProcessorValueTreeState::ParameterLayout AudioPluginAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> parameters;

    parameters.push_back(std::make_unique<juce::AudioParameterChoice>("DEVICES", "Devices", AudioPluginConstants::devices, 0));
    parameters.push_back(std::make_unique<juce::AudioParameterChoice>("MODELS", "Models", AudioPluginConstants::models, 0));
    parameters.push_back(std::make_unique<juce::AudioParameterInt>("PORT", "Port", 0, 65535, 8000));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>("AUDIO_LENGTH", "Audio Length", 1.0f, 30.0f, AudioPluginConstants::initialAudioLength));
    parameters.push_back(std::make_unique<juce::AudioParameterInt>("NUM_INFERENCE_STEPS", "Number of Inference Steps", 5,20, AudioPluginConstants::initialNumInference));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>("GUIDANCE_SCALE", "Guidance Scale", 1.0f, 5.0f, AudioPluginConstants::initialGuidanceScale));
    parameters.push_back(std::make_unique<juce::AudioParameterBool>("AUTO_START_SERVER", "Auto Start Server", AudioPluginConstants::initialAutoStartServer));
    parameters.push_back(std::make_unique<juce::AudioParameterBool>("AUTO_SETUP_MODEL", "Auto Setup Model", AudioPluginConstants::initialAutoModelSetup));



    return { parameters.begin(), parameters.end() };
}


AudioPluginAudioProcessor::AudioPluginAudioProcessor()
        : MagicProcessor (BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
                                  .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
                                  .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
),
          apvts(*this, nullptr, "PARAMETERS", createParameterLayout())
{
    FOLEYS_SET_SOURCE_PATH(__FILE__);
    magicState.setGuiValueTree(BinaryData::magic_xml, BinaryData::magic_xmlSize);
    apiHandler = std::make_unique<ApiHandler>(*this);
    guiHandler = std::make_unique<GuiHandler>(*this, magicState);
    setupProcessor();

}

AudioPluginAudioProcessor::~AudioPluginAudioProcessor() = default;

void AudioPluginAudioProcessor::setupProcessor()
{
    setReferenceValues();
    registerEventTriggers();
    apiHandler->initializeApiConnection(isAutoStartServer(), isAutoModelSetup() );
}

void AudioPluginAudioProcessor::registerEventTriggers()
{
    magicState.addTrigger("generate", [&] { generateSampleFromPrompt(); });
    magicState.addTrigger("refresh", [&] { refresh(); });
    magicState.addTrigger("startServer", [&] { startServer(); });
    magicState.addTrigger("initModel", [&] { initModel(); });

}

void AudioPluginAudioProcessor::setReferenceValues()
{
    promptValue.referTo(magicState.getPropertyAsValue("prompt"));
    negativePromptValue.referTo(magicState.getPropertyAsValue("negative_prompt"));
}

void AudioPluginAudioProcessor::refresh() const
{
    Logger::logInfo("Refresh");
    apiHandler->fetchStatusAndParameters();
}
bool AudioPluginAudioProcessor::isAutoStartServer() {
    bool val =  magicState.getPropertyAsValue("auto_start").getValue();
    if(val) return true;
    return false;
}
bool AudioPluginAudioProcessor::isAutoModelSetup() {
    bool val = magicState.getPropertyAsValue("auto_setup").getValue();
    if(val) return true;
    return false;
}
void AudioPluginAudioProcessor::startServer() {
    Logger::logInfo("(RE)-Starting server");
    int port = apvts.getParameter("PORT")->getCurrentValueAsText().getIntValue();
    bool autoStart = magicState.getPropertyAsValue("auto_start").getValue();
    std::cout << "AutoStart: " << autoStart << std::endl;
    std::cout << "Port: " << port << std::endl;

    apiHandler->startServer(port);
    //TODO
}

juce::String AudioPluginAudioProcessor::getModelProperty() const {
    return apvts.getParameter("MODELS")->getCurrentValueAsText();
}

juce::String AudioPluginAudioProcessor::getDeviceProperty() const {
    return apvts.getParameter("DEVICES")->getCurrentValueAsText();
}

int AudioPluginAudioProcessor::getPort() const {
    return apvts.getParameter("PORT")->getCurrentValueAsText().getIntValue();
}

void AudioPluginAudioProcessor::initModel() const {
    Logger::logInfo("(RE)-Initializing model");
    apiHandler->initModel(getDeviceProperty(), getModelProperty());
}


void:: AudioPluginAudioProcessor::generateSampleFromPrompt() {
    apiHandler->generateSample(
            magicState.getPropertyAsValue("prompt").toString(),
            magicState.getPropertyAsValue("negative_prompt").toString(),
            apvts.getParameter("AUDIO_LENGTH")->getCurrentValueAsText().getFloatValue(),
            apvts.getParameter("NUM_INFERENCE_STEPS")->getCurrentValueAsText().getIntValue(),
            apvts.getParameter("GUIDANCE_SCALE")->getCurrentValueAsText().getFloatValue()
    );
}

const juce::String AudioPluginAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AudioPluginAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool AudioPluginAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool AudioPluginAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double AudioPluginAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int AudioPluginAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int AudioPluginAudioProcessor::getCurrentProgram()
{
    return 0;
}

void AudioPluginAudioProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused (index);
}

const juce::String AudioPluginAudioProcessor::getProgramName (int index)
{
    juce::ignoreUnused (index);
    return {};
}

void AudioPluginAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

//==============================================================================
void AudioPluginAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    juce::ignoreUnused (sampleRate, samplesPerBlock);
    magicState.getPropertyAsValue ("prompt").setValue(AudioPluginConstants::initialPromptFieldMessage);
    magicState.getPropertyAsValue ("negative_prompt").setValue(AudioPluginConstants::initialNegativePromptFieldMessage);
    magicState.getPropertyAsValue ("auto_setup").setValue(AudioPluginConstants::initialAutoModelSetup);
    magicState.getPropertyAsValue ("auto_start").setValue(AudioPluginConstants::initialAutoStartServer);
}

void AudioPluginAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool AudioPluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
#else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
#if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
#endif
}

void AudioPluginAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                              juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);

    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);
        juce::ignoreUnused (channelData);
        // ..do something to the data...
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AudioPluginAudioProcessor();
}

