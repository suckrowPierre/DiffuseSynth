//
// Created by Pierre-Louis Suckrow on 12.07.23.
//

#ifndef DIFFUSESYNTH_APICLIENTHELPER_H
#define DIFFUSESYNTH_APICLIENTHELPER_H

#include "juce_audio_processors/juce_audio_processors.h"
#include "AudioLDMApiClient.h"

class AudioPluginAudioProcessor;

class ApiHandler
{
public:
    explicit ApiHandler(AudioPluginAudioProcessor& processor);
    ~ApiHandler() = default;

    void initializeApiConnection(bool autoStartServer, bool autoSetupModel);
    void generateSample(juce::String prompt, juce::String negative_prompt, float audio_length_in_s, int num_inference_steps, float guidance_scale);
    void initModel(juce::String device, juce::String repo_id);
    void fetchStatusAndParameters();
    void startServer(int& port);



private:
    AudioPluginAudioProcessor& processor;
    std::unique_ptr<AudioLDMApiClient> apiClient;

    void autoStartServer();
    void autoInitModel();

    bool checkApiStatus();
    bool checkModelStatus();
    void getCurrentParams();
    [[nodiscard]] bool isApiClientInitialized() const;
};


#endif //DIFFUSESYNTH_APICLIENTHELPER_H
