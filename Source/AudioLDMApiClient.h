/**
 * @file AudioLDMApiClient.h
 * @brief Defines the AudioLDMApiClient class and related parameter classes for audio API.
 *
 * @author Pierre-Louis Suckrow
 * @date 06/07/2023
 */

#ifndef DIFFUSESYNTH_AUDIOLDMAPICLIENT_H
#define DIFFUSESYNTH_AUDIOLDMAPICLIENT_H

#include <juce_audio_processors/juce_audio_processors.h>


class SetupModelParameters {
public:
    juce::String device;
    juce::String repo_id;
};

class GenerateSampleParameters {
public:
    juce::String prompt;
    juce::String negative_prompt;
};

class AudioLDMApiClient {
private:
    juce::String apiHost = "http://localhost";
    juce::String apiPort;
    juce::String setupEndpointPath = "/setup";
    juce::String generateSampleEndpointPath = "/generate";

    [[nodiscard]] static juce::String getSetupBody(const SetupModelParameters& params) ;
    [[nodiscard]] static juce::String getGenerateSampleBody(const GenerateSampleParameters& params) ;
    [[nodiscard]] static juce::var sendPostRequest(const juce::URL& url, const juce::String& body) ;
    static juce::var handleApiResponse(juce::WebInputStream& stream) ;
    [[nodiscard]] juce::String extractAudioDataFromResponse(const juce::String &body, const juce::URL &url) const;
    static void decodeAudio(const juce::String& audioBase64) ;
    [[nodiscard]] juce::String constructApiUrl(const juce::String& endpointPath) const;

public:
    /**
     * @brief Sets up the model using the specified parameters.
     *
     * @param params The setup parameters.
     * @return true on success, false otherwise.
     * @throws std::invalid_argument if device or repo_id is empty.
     */
    bool setupModel(const SetupModelParameters& params);
    /**
     * @brief Generates a sample with the specified parameters.
     *
     * @param params The generation parameters.
     * @return true on success, false otherwise.
     * @throws std::invalid_argument if the prompt is empty.
     */
    bool generateSample(const GenerateSampleParameters& params);
    /**
     * @brief Sets the port to be used in API calls.
     *
     * @param port The port to be used.
     */
    void setApiPort(const juce::String& port);
};


#endif //DIFFUSESYNTH_AUDIOLDMAPICLIENT_H
