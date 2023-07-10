#include "PluginProcessor.h"
#include "BinaryData.h"


//LocalizationManager* _manager = &LocalizationManager::getInstance();

//==============================================================================
AudioPluginAudioProcessor::AudioPluginAudioProcessor()
        : MagicProcessor (BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
                                  .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
#endif
                                  .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
#endif
),
          apiClient(std::make_unique<AudioLDMApiClient>())
{

    FOLEYS_SET_SOURCE_PATH(__FILE__);
    magicState.setGuiValueTree(BinaryData::magic_xml, BinaryData::magic_xmlSize);

    promptValue.referTo(magicState.getPropertyAsValue("prompt"));
    negativePromptValue.referTo(magicState.getPropertyAsValue("negative_prompt"));
    audioLengthValue.referTo(magicState.getPropertyAsValue("audio_length_in_s"));
    numInferenceValue.referTo(magicState.getPropertyAsValue("num_inference_steps"));
    guidanceScaleValue.referTo(magicState.getPropertyAsValue("guidance_scale"));


    magicState.addTrigger("generate", [&] {
        generateSampleFromPrompt();
    });

    magicState.addTrigger("refresh", [&] {
        refresh();
    });

    connectToApi();
}

AudioPluginAudioProcessor::~AudioPluginAudioProcessor() {
}

//==============================================================================

void:: AudioPluginAudioProcessor::connectToApi() {
    if (apiClient) {
        apiClient->setApiPort("8000");
        try {
            if (checkApiStatus()) {
                juce::var devicesAndModels = apiClient->getSetupParameters();
                extractDeviceAndModelParameters(devicesAndModels);

                if (!checkModelStatus()) {
                    SetupModelParameters params;
                    //TODO: get device and repo_id from saved state
                    params.device = "mps";
                    params.repo_id = "audioldm-l-full";
                    juce::Logger::writeToLog("Setting up model");
                    if(apiClient->setupModel(params)) {
                        juce::Logger::writeToLog("Model setup");
                    }

                } else {
                    juce::Logger::writeToLog("Model setup");
                }


            }
        } catch (const std::exception& e) {
            juce::Logger::writeToLog("Failed to connect to API. Exception: " + juce::String(e.what()));
        }
    } else {
        juce::Logger::writeToLog("Failed to connect to API. AudioLDMApiClient not initialized.");
    }
    refresh();
}

void ::AudioPluginAudioProcessor::refresh() {
    checkApiStatus();
    checkModelStatus();
}

bool ::AudioPluginAudioProcessor::checkModelStatus(){

    try {
        bool isAvailable = apiClient->isModelSetUp();
        if (isAvailable) {
            juce::Logger::writeToLog("Model is available");
            updateGUIStatus(GUIModelStatusId, green);
            return isAvailable;
        }
    }
    catch (const std::exception& e) {
    }
    juce::Logger::writeToLog("Model is not available");
    updateGUIStatus(GUIModelStatusId,red);
    return false;
}

bool ::AudioPluginAudioProcessor::checkApiStatus(){
    bool isAvailable = apiClient->isApiAvailable();
    if (!isAvailable) {
        juce::Logger::writeToLog("API is not available");
        updateGUIStatus(GUIServerStatusId,red);
        return false;
    }
    juce::Logger::writeToLog("API is available");
    updateGUIStatus(GUIServerStatusId, green);
    return true;
}

juce::ValueTree AudioPluginAudioProcessor::getNodeById(const juce::String& id)
{
    if(nodePaths.count(id) == 0) // If the path for this id is not stored
    {
        juce::Identifier identifier = juce::Identifier("id");
        auto guiTree = magicState.getGuiTree().getChild(1);
        nodePaths[id] = searchGUITreeRecursively(guiTree, identifier, id); // Store the path
    }

    return getNodeByPath(nodePaths[id]); // Retrieve the node by the stored path
}

std::vector<int> AudioPluginAudioProcessor::searchGUITreeRecursively(const juce::ValueTree& node, const juce::Identifier& identifier, const juce::String& id, std::vector<int> path)
{
    int numberChildren = node.getNumChildren();
    for(int i = 0; i < numberChildren; i++)
    {
        auto child = node.getChild(i);
        if(child.isValid() && child.hasProperty(identifier) && child.getProperty(identifier) == id)
        {
            path.push_back(i);
            return path;
        }
        else
        {
            auto newPath = path;
            newPath.push_back(i);
            auto result = searchGUITreeRecursively(child, identifier, id, newPath);
            if(!result.empty())
            {
                return result;
            }
        }
    }

    return {}; // Return an empty vector if no matching node was found
}

juce::ValueTree AudioPluginAudioProcessor::getNodeByPath(const std::vector<int>& path)
{
    auto node = magicState.getGuiTree().getChild(1); // Start from the initial node
    for(auto index : path)
    {
        if(node.getNumChildren() > index)
            node = node.getChild(index);
        else
            return {}; // Return an invalid ValueTree if the path does not exist
    }

    return node;
}


void ::AudioPluginAudioProcessor::fillComboBox(juce::Array<juce::var>& items, const juce::String& id){
    auto comboBox = getNodeById(id);

    if (!comboBox.isValid()) {
        juce::Logger::writeToLog("Failed to find node with id: " + id);
        return;
    }

    std::make_unique<juce::AudioParameterChoice>(juce::ParameterID ("models", 1), "Model", items, 0);
    //TODO

    //set the parameter to the combobox
    comboBox.setProperty (juce::Identifier ("parameter"), juce::var("models"), nullptr);

}

void ::AudioPluginAudioProcessor::updateGUIStatus(const juce::String& id, const juce::String& colour){


    auto node = getNodeById(id);

    if (!node.isValid()) {
        juce::Logger::writeToLog("Failed to find node with id: " + id);
        return;
    }
    juce::var colourValue = colour;
    node.setProperty (juce::Identifier ("label-text"), colourValue, nullptr);
}

void ::AudioPluginAudioProcessor::extractDeviceAndModelParameters(const juce::var &devicesAndModels) {
    juce::Array<juce::var> devices = *devicesAndModels.getProperty("devices", juce::var()).getArray();
    juce::Array<juce::var> models = *devicesAndModels.getProperty("models", juce::var()).getArray();

    juce::Logger::writeToLog("Devices:");
    for (auto& device : devices) {
        juce::Logger::writeToLog(device.toString());
    }

    fillComboBox(devices, "device_list");

    juce::Logger::writeToLog("Models:");
    for (auto& model : models) {
        juce::Logger::writeToLog(model.toString());
    }
    //TODO write devies and models to GUI
}

void:: AudioPluginAudioProcessor::generateSampleFromPrompt() {

    if(apiClient) {

        GenerateSampleParameters params;
        params.prompt = magicState.getPropertyAsValue("prompt").toString();
        params.negative_prompt = magicState.getPropertyAsValue("negative_prompt").toString();
        params.audio_length_in_s = magicState.getPropertyAsValue("audio_length_in_s").getValue();
        params.num_inference_steps = magicState.getPropertyAsValue("num_inference_steps").getValue();
        params.guidance_scale = magicState.getPropertyAsValue("guidance_scale").getValue();


        juce::Logger::writeToLog("Generating sample with parameters: " + params.prompt + ", " + params.negative_prompt + ", " + juce::String(params.audio_length_in_s) + ", " + juce::String(params.num_inference_steps) + ", " + juce::String(params.guidance_scale));

        try {
            if (apiClient->generateSample(params)){
                juce::Logger::writeToLog("Sample generated");
            }

        } catch (const std::exception& e) {
            juce::Logger::writeToLog("Failed to generate sample. Exception: " + juce::String(e.what()));
        }
    }else {
        juce::Logger::writeToLog("Failed to connect to API. AudioLDMApiClient not initialized.");
    }
}

void:: AudioPluginAudioProcessor::setupModel(juce::String device, juce::String repo_id) {
    if (apiClient) {
        SetupModelParameters params;
        params.device = device;
        params.repo_id = repo_id;


        try {
            if (apiClient->setupModel(params)){
                juce::Logger::writeToLog("successfully set up model");
            }
        } catch (const std::exception& e) {
            juce::Logger::writeToLog("Failed to set up model. Exception: " + juce::String(e.what()));
        }
    } else {
        juce::Logger::writeToLog("Failed to set up model. AudioLDMApiClient not initialized.");
    }
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
    magicState.getPropertyAsValue ("prompt").setValue(initialPromptFieldMessage);
    magicState.getPropertyAsValue ("negative_prompt").setValue(initialNegativePromptFieldMessage);
    magicState.getPropertyAsValue("audio_length_in_s").setValue(initialAudioLength);
    magicState.getPropertyAsValue("num_inference_steps").setValue(initialNumInference);
    magicState.getPropertyAsValue("guidance_scale").setValue(initialGuidanceScale);

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
