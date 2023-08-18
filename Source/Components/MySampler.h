//
// Created by Pierre-Louis Suckrow on 18.08.23.
//

#ifndef DIFFUSESYNTH_MYSAMPLERVOICE_H
#define DIFFUSESYNTH_MYSAMPLERVOICE_H

#include <juce_audio_formats/juce_audio_formats.h>

class  MySamplerSound    : public juce::SynthesiserSound
{
public:
    //==============================================================================
    /** Creates a sampled sound from an audio reader.

        This will attempt to load the audio from the source into memory and store
        it in this object.

        @param name         a name for the sample
        @param source       the audio to load. This object can be safely deleted by the
                            caller after this constructor returns
        @param midiNotes    the set of midi keys that this sound should be played on. This
                            is used by the SynthesiserSound::appliesToNote() method
        @param midiNoteForNormalPitch   the midi note at which the sample should be played
                                        with its natural rate. All other notes will be pitched
                                        up or down relative to this one
        @param attackTimeSecs   the attack (fade-in) time, in seconds
        @param releaseTimeSecs  the decay (fade-out) time, in seconds
        @param maxSampleLengthSeconds   a maximum length of audio to read from the audio
                                        source, in seconds
    */
    MySamplerSound (const juce::String& name,
                    juce::AudioFormatReader& source,
                  const juce::BigInteger& midiNotes,
                  int midiNoteForNormalPitch,
                  double attackTimeSecs,
                  double releaseTimeSecs,
                  double maxSampleLengthSeconds);

    /** Destructor. */
    ~MySamplerSound() override;

    //==============================================================================
    /** Returns the sample's name */
    const juce::String& getName() const noexcept                  { return name; }

    /** Returns the audio sample data.
        This could return nullptr if there was a problem loading the data.
    */
    juce::AudioBuffer<float>* getAudioData() const noexcept       { return data.get(); }

    //==============================================================================
    /** Changes the parameters of the ADSR envelope which will be applied to the sample. */
    void setEnvelopeParameters (juce::ADSR::Parameters parametersToUse)    { params = parametersToUse; }

    //==============================================================================
    bool appliesToNote (int midiNoteNumber) override;
    bool appliesToChannel (int midiChannel) override;

private:
    //==============================================================================
    friend class MySamplerVoice;

    juce::String name;
    std::unique_ptr<juce::AudioBuffer<float>> data;
    double sourceSampleRate;
    juce::BigInteger midiNotes;
    int
            length = 0, midiRootNote = 0;

    juce::ADSR::Parameters params;

    JUCE_LEAK_DETECTOR (MySamplerSound)
};

class MySamplerVoice:  public juce::SamplerVoice{
public:
    MySamplerVoice();
    ~MySamplerVoice() override;

    void setPitch(float pitch) {
        this->pitchShift = pitch;
    }
    //==============================================================================
    bool canPlaySound (juce::SynthesiserSound*) override;

    void startNote (int midiNoteNumber, float velocity, juce::SynthesiserSound*, int pitchWheel) override;
    void stopNote (float velocity, bool allowTailOff) override;

    void pitchWheelMoved (int newValue) override;
    void controllerMoved (int controllerNumber, int newValue) override;

    void renderNextBlock (juce::AudioBuffer<float>&, int startSample, int numSamples) override;
    using SynthesiserVoice::renderNextBlock;
private:
    float pitchShift = 1;
    double pitchRatio = 0;
    double basePitchRatio = 0;
    double sourceSamplePosition = 0;
    float lgain = 0, rgain = 0;

    juce::ADSR adsr;

    JUCE_LEAK_DETECTOR (MySamplerVoice)
};


#endif //DIFFUSESYNTH_MYSAMPLERVOICE_H
