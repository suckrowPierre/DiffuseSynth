/**
 * @file AudioLDMApiClient.cpp
 * @brief Implements the AudioLDMApiClient class and related classes for interacting with an audio API.
 *
 * @author Pierre-Louis Suckrow
 * @date 06/07/2023
 */


#include "AudioLDMApiClient.h"
#include <stdexcept>
#include <string>
#include "../Util/Constants.h"


/**
 * Sets up the model using the specified parameters.
 * @param params The setup parameters.
 * @return true on success, false otherwise.
 * @throws std::invalid_argument if device or repo_id is empty.
 */
bool AudioLDMApiClient::setupModel(const SetupModelParameters& params) {
    if (params.device.isEmpty() || params.repo_id.isEmpty()) {
        throw std::invalid_argument("Device and repo_id must not be empty");
    }

    juce::String body = getSetupBody(params);
    juce::URL url = constructApiUrl(setupEndpointPath);
    juce::var response = sendPostRequest(url, body);
    return true;
}

/**
 * Generates a sample with the specified parameters.
 * @param params The generation parameters.
 * @return true on success, false otherwise.
 * @throws std::invalid_argument if the prompt is empty.
 */
bool AudioLDMApiClient::generateSample(const GenerateSampleParameters& params) {
    if (params.prompt.isEmpty()) {
        throw std::invalid_argument("Prompt must not be empty");
    }

    juce::String body = getGenerateSampleBody(params);
    std::cout << body << std::endl;
    juce::URL url = constructApiUrl(generateSampleEndpointPath);
    juce::String base64AudioData = extractAudioDataFromResponse(body, url);
    decodeAudio(base64AudioData);
    return true;
}

bool AudioLDMApiClient::isApiAvailable() const{
    juce::URL url = constructApiUrl("/");
    try {
        sendGetRequest(url);
    } catch (const std::exception& e) {
        return false;
    }
    return true;
}

juce::var AudioLDMApiClient::getCurrentParameters() const{
    juce::URL url = constructApiUrl(currentParametersEndpointPath);
    juce::var response = sendGetRequest(url);

    if (!response.isObject()){
        throw std::runtime_error("Response is not a JSON object");
    }

    return response;
}

bool AudioLDMApiClient::isModelSetUp() const {
    juce::URL url = constructApiUrl(modelStatusEndpointPath);
    juce::var response = sendGetRequest(url);
    return static_cast<bool>(response["isModelSetup"]);
}


/**
 * Extracts the audio data from the response of an API call.
 * @param body The body of the API call.
 * @param url The API call URL.
 * @return The audio data as a base64-encoded string.
 * @throws std::runtime_error if there's a problem with the API call.
 */
juce::String AudioLDMApiClient::extractAudioDataFromResponse(const juce::String &body, const juce::URL &url) const {
    juce::var response = sendPostRequest(url, body);
    return response["audio"].toString();
}

/**
 * Sends a POST request to the specified URL.
 * @param url The URL to send the request to.
 * @param body The body of the request.
 * @return The response from the server.
 * @throws std::runtime_error if there's a problem with the connection.
 */
juce::var AudioLDMApiClient::sendPostRequest(const juce::URL& url, const juce::String& body) {
    juce::WebInputStream stream(url.withPOSTData(body), true);
    stream.withExtraHeaders("Content-Type: application/json");

    if(!stream.connect(nullptr)) {
        throw std::runtime_error("Failed to connect to API");
    }

    return handleApiResponse(stream);
}


juce::var AudioLDMApiClient::sendGetRequest(const juce::URL& url) {
    juce::WebInputStream stream(url, true);
    stream.withCustomRequestCommand("GET");

    if(!stream.connect(nullptr)) {
        throw std::runtime_error("Failed to connect to API");
    }

    return handleApiResponse(stream);
}

/**
 * Handles the response from an API call.
 * @param stream The stream to read the response from.
 * @return The parsed response.
 * @throws std::runtime_error if the response status code isn't 200.
 */
juce::var AudioLDMApiClient::handleApiResponse(juce::WebInputStream& stream) {
    int statusCode = stream.getStatusCode();
    juce::String response = stream.readString();
    if (statusCode != 200) {
        juce::var jsonResponse = juce::JSON::parse(response);
        std::string detail = jsonResponse["detail"].toString().toStdString();
        throw std::runtime_error(detail);
    }
    return juce::JSON::parse(response);
}

/**
 * Decodes base64-encoded audio data.
 * @param audioBase64 The base64-encoded audio data.
 * @throws std::runtime_error if the audio data is empty or if there's a problem with decoding or writing the audio file.
 */
void AudioLDMApiClient::decodeAudio(const juce::String& audioBase64) {
    if (audioBase64.isEmpty() || audioBase64 == "") {
        throw std::runtime_error("audioBase64 is empty");
    }
    juce::MemoryOutputStream decodedAudio;
    if (!juce::Base64::convertFromBase64 (decodedAudio, audioBase64)) {
        throw std::runtime_error("Failed to decode audio");
    }

    juce::File tempFile = juce::File::getSpecialLocation (juce::File::SpecialLocationType::tempDirectory)
            .getChildFile (AudioPluginConstants::tempFileName);


    if (tempFile.exists())
    {
        bool wasDeleted = tempFile.deleteFile();
        if (!wasDeleted) {
            throw std::runtime_error("Failed to delete existing file");
        }
    }

    std::unique_ptr<juce::FileOutputStream> fos (tempFile.createOutputStream());

    if (fos == nullptr || fos->failedToOpen()) {
        throw std::runtime_error("Failed to open file");
    }

    fos->write (decodedAudio.getData(), decodedAudio.getDataSize());
    fos->flush();
    fos.reset();

    juce::Logger::writeToLog("File written to " + tempFile.getFullPathName());
}


/**
 * Constructs the full API URL using the host, port, and endpoint path.
 * @param endpointPath The endpoint path.
 * @return The full API URL.
 * @throws std::invalid_argument if the host, port, or endpoint path is empty.
 */
juce::String AudioLDMApiClient::constructApiUrl(const juce::String& endpointPath) const {
    if(apiHost.isEmpty()) {
        throw std::invalid_argument("Api port must not be empty");
    }
    if(apiHost.isEmpty()) {
        throw std::invalid_argument("Api host must not be empty");
    }
    if(endpointPath.isEmpty()) {
        throw std::invalid_argument("Endpoint path must not be empty");
    }
    return apiHost + ":" + apiPort + endpointPath;
}

/**
 * Sets the port to be used in API calls.
 * @param port The port to be used.
 */
void AudioLDMApiClient::setApiPort(const juce::String& port) {
    apiPort = port;
}

/**
 * Constructs the body for the setup model API call.
 * @param params The setup parameters.
 * @return The request body as a JSON string.
 */
juce::String AudioLDMApiClient::getSetupBody(const SetupModelParameters& params) {
    juce::String body = R"({ "device": ")" + params.device + R"(", "repo_id": ")" + params.repo_id + "\" }";
    return body;
}

/**
 * Constructs the body for the generate sample API call.
 * @param params The generation parameters.
 * @return The request body as a JSON string.
 */
juce::String AudioLDMApiClient::getGenerateSampleBody(const GenerateSampleParameters& params) {
    juce::String body = R"({ "prompt": ")" + params.prompt + R"(", "negative_prompt": ")" + params.negative_prompt + R"(", "audio_length_in_s": ")" + std::to_string(params.audio_length_in_s) + R"(", "num_inference_steps": ")" + std::to_string(params.num_inference_steps) + R"(", "guidance_scale": ")" + std::to_string(params.guidance_scale) + + R"(", "sample_rate": ")" + std::to_string(params.sample_rate) + "\" }";
    return body;
}

juce::String AudioLDMApiClient::getApiPort() const {
    if (apiPort.isEmpty()) {
        return "";
    }
    return apiPort;
}
