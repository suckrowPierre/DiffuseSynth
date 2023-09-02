#include "PluginProcessor.h"
#include "BinaryData.h"
#include "Util/GuiHandler.h"
#include "Util/Logger.h"
#include <string>


void AudioPluginAudioProcessor::addChoiceParameters(std::vector<std::unique_ptr<juce::RangedAudioParameter>>& parameters) {
    parameters.push_back(std::make_unique<juce::AudioParameterChoice>("DEVICES", "Devices", AudioPluginConstants::devices, 0));
    parameters.push_back(std::make_unique<juce::AudioParameterChoice>("MODELS", "Models", AudioPluginConstants::models, 0));
}

void AudioPluginAudioProcessor::addIntParameters(std::vector<std::unique_ptr<juce::RangedAudioParameter>>& parameters) {
    parameters.push_back(std::make_unique<juce::AudioParameterInt>("PORT", "Port", 0, 65535, 8000));
    parameters.push_back(std::make_unique<juce::AudioParameterInt>("NUM_INFERENCE_STEPS", "Number of Inference Steps", 5, 20, AudioPluginConstants::initialNumInference));
}

void AudioPluginAudioProcessor::addBoolParameters(std::vector<std::unique_ptr<juce::RangedAudioParameter>>& parameters) {
    parameters.push_back(std::make_unique<juce::AudioParameterBool>("AUTO_START_SERVER", "Auto Start Server", AudioPluginConstants::initialAutoStartServer));
}

void AudioPluginAudioProcessor::addFloatParameters(std::vector<std::unique_ptr<juce::RangedAudioParameter>>& parameters) {
    addFloatParameter(parameters, "AUDIO_LENGTH", "Audio Length", 1.0f, 30.0f, AudioPluginConstants::initialAudioLength);
    addFloatParameter(parameters, "GUIDANCE_SCALE", "Guidance Scale", 1.0f, 5.0f,
                      AudioPluginConstants::initialGuidanceScale);
    addFloatParameter(parameters, "PITCH", "Pitch", AudioPluginConstants::minPitch, AudioPluginConstants::maxPitch, 0);
    addFloatParameter(parameters, "ATTACK_", "Attack", AudioPluginConstants::minAttack, AudioPluginConstants::maxAttack,
                      AudioPluginConstants::defaultAttack);
    addFloatParameter(parameters, "DECAY_", "Decay", AudioPluginConstants::minDecay, AudioPluginConstants::maxDecay,
                      AudioPluginConstants::defaultDecay);
    addFloatParameter(parameters, "SUSTAIN_", "Sustain", AudioPluginConstants::minSustain,
                      AudioPluginConstants::maxSustain, AudioPluginConstants::defaultSustain);
    addFloatParameter(parameters, "RELEASE_", "Release", AudioPluginConstants::minRelease,
                      AudioPluginConstants::maxRelease, AudioPluginConstants::defaultRelease);
    addFloatParameter(parameters, "GAIN", "Gain", AudioPluginConstants::minGain, AudioPluginConstants::maxGain, 0.7f);

}

juce::AudioProcessorValueTreeState::ParameterLayout AudioPluginAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> parameters;
    addBoolParameters(parameters);
    addChoiceParameters(parameters);
    addIntParameters(parameters);
    addFloatParameters(parameters);
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
    //FOLEYS_SET_SOURCE_PATH(__FILE__);
    magicState.setGuiValueTree(BinaryData::magic_xml, BinaryData::magic_xmlSize);

    magicState.getPropertyAsValue ("prompt").setValue(AudioPluginConstants::initialPromptFieldMessage);
    magicState.getPropertyAsValue ("negative_prompt").setValue(AudioPluginConstants::initialNegativePromptFieldMessage);
    magicState.getPropertyAsValue ("seed").setValue("");
    magicState.getPropertyAsValue ("auto_setup").setValue( AudioPluginConstants::initialAutoModelSetup);
    magicState.getPropertyAsValue ("auto_start").setValue(AudioPluginConstants::initialAutoStartServer);



    outputMeter = magicState.createAndAddObject<foleys::MagicLevelSource>("output");
    formatManager.registerBasicFormats();
    audioThumbnail = magicState.createAndAddObject<foleys::SampleHolder>("Waveform", thumbnailCache, formatManager);
    apiHandler = std::make_unique<ApiHandler>(*this);
    guiHandler = std::make_unique<GuiHandler>(*this, magicState);
    setupProcessor();

    for(int i = 0; i < AudioPluginConstants::numVoices; i++) {
        sampler.addVoice(new MySamplerVoice());
    }

    pitch = dynamic_cast<juce::AudioParameterFloat *>(getParameter("PITCH"));
    gain = dynamic_cast<juce::AudioParameterFloat *>(getParameter("GAIN"));
    attack = dynamic_cast<juce::AudioParameterFloat *>(getParameter("ATTACK_"));
    decay = dynamic_cast<juce::AudioParameterFloat *>(getParameter("DECAY_"));
    sustain = dynamic_cast<juce::AudioParameterFloat *>(getParameter("SUSTAIN_"));
    release = dynamic_cast<juce::AudioParameterFloat *>(getParameter("RELEASE_"));
}



AudioPluginAudioProcessor::~AudioPluginAudioProcessor() {
        reader = nullptr;
        juce::File file(juce::File::getSpecialLocation(juce::File::SpecialLocationType::tempDirectory)
                            .getChildFile(AudioPluginConstants::tempFileName));
        file.deleteFile();
}

void AudioPluginAudioProcessor::addFloatParameter(std::vector<std::unique_ptr<juce::RangedAudioParameter>>& parameters,
                                             const std::string& id, const std::string& name,
                                             float min, float max, float defaultVal) {
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(id, name, min, max, defaultVal));
}


juce::RangedAudioParameter* AudioPluginAudioProcessor::getParameter(const std::string& paramId) {
    auto param = dynamic_cast<juce::RangedAudioParameter*>(apvts.getParameter(paramId));
    jassert(param);
    return param;
}

void AudioPluginAudioProcessor::fetchADSRParameters(){
    ADSRParameters.attack = attack->get();
    ADSRParameters.decay = decay->get();
    ADSRParameters.sustain = sustain->get();
    ADSRParameters.release = release->get();
}

void AudioPluginAudioProcessor::updateADSRParameters() {
    fetchADSRParameters();

    for(int i = 0; i < sampler.getNumSounds(); i++) {
        auto* sound = dynamic_cast<MySamplerSound*>(sampler.getSound(i).get());
        if(sound) {;
            sound->setEnvelopeParameters(ADSRParameters);
        }
    }
}

void AudioPluginAudioProcessor::updatePitch()
{

    float semitoneShift = juce::roundToInt(pitch->get()*100)*0.01f;
    float pitchShift = std::pow(2.0f, semitoneShift / 12.0f);
    for (int i = 0; i < sampler.getNumVoices(); ++i)
    {
        if (auto* voice = dynamic_cast<MySamplerVoice*>(sampler.getVoice(i)))
        {
            voice->setTuneRatio(pitchShift);
        }
    }

}

void AudioPluginAudioProcessor::initialiseBuilder (foleys::MagicGUIBuilder& builder)
{
    std::cout << "initialiseBuilder" << std::endl;
    builder.registerJUCEFactories();
    builder.registerJUCELookAndFeels();

    builder.registerFactory("WaveformDisplay", &foleys::WaveformItem::factory);
    builder.registerFactory("SpectrogramDisplay", &foleys::SpectrogramItem::factory);



}


void AudioPluginAudioProcessor::loadFile(){
    Logger::logInfo("Loading file");

    juce::File file(juce::File::getSpecialLocation(juce::File::SpecialLocationType::tempDirectory)
                            .getChildFile(AudioPluginConstants::tempFileName));

    Logger::logInfo("path: " + file.getFullPathName());

    if (file.existsAsFile())
    {
        std::unique_ptr<juce::AudioFormatReader> readerPtr(formatManager.createReaderFor(file));

        if (readerPtr)
        {
            juce::BigInteger range;
            range.setRange(0, 128, true);

            auto sampleLength = static_cast<int>(readerPtr->lengthInSamples);
            waveForm.setSize(1, sampleLength);
            readerPtr->read(&waveForm, 0, sampleLength, 0, true, true);

            audioThumbnail->setAudioFileAndBuffer(file, &waveForm);

            sampler.addSound(new MySamplerSound("sample", *readerPtr, range, 60, 0, 0.1, 10.0));
            updateADSRParameters();
            updatePitch();
        }
        else
        {
            Logger::logError("Unable to create reader for file");
        }
    }
    else
    {
        Logger::logError("File does not exist");
    }
}

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
    seedValue.referTo(magicState.getPropertyAsValue("seed"));
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
    try {
        apiHandler->generateSample(
                magicState.getPropertyAsValue("prompt").toString(),
                magicState.getPropertyAsValue("negative_prompt").toString(),
                apvts.getParameter("AUDIO_LENGTH")->getCurrentValueAsText().getFloatValue(),
                apvts.getParameter("NUM_INFERENCE_STEPS")->getCurrentValueAsText().getIntValue(),
                apvts.getParameter("GUIDANCE_SCALE")->getCurrentValueAsText().getFloatValue(),
                validateSeed(magicState.getPropertyAsValue("seed").toString())
        );
        std::cout << "test" << std::endl;
        loadFile();
    } catch (const std::exception& e) {
        Logger::logException(e);
    }
}

juce::String AudioPluginAudioProcessor::validateSeed(const juce::String &seedString) {
    if(seedString != ""){
        unsigned long int seed;
        try {
            seed = std::stoul(seedString.toStdString(), nullptr, 0);
        }
        catch (const std::exception &e) {
            magicState.getPropertyAsValue("seed").setValue("invalid seed");
            throw std::invalid_argument("Invalid seed");
        }
        return juce::String(seed);
    }
    return "";
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
    sampler.setCurrentPlaybackSampleRate(sampleRate);
    outputMeter->setupSource(getTotalNumOutputChannels(), sampleRate, AudioPluginConstants::maxKeepMS);
    //magicState.prepareToPlay(sampleRate, samplesPerBlock);

    updateADSRParameters();
    updatePitch();
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
    //juce::ignoreUnused (midiMessages);

    //juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    updatePitch();
    updateADSRParameters();
    sampler.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());

    buffer.applyGain(gain->get());
    outputMeter->pushSamples(buffer);

}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AudioPluginAudioProcessor();
}

