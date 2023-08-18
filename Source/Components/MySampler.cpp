//
// Created by Pierre-Louis Suckrow on 18.08.23.
//

#include "MySampler.h"

    MySamplerSound::MySamplerSound(const juce::String &soundName,
                                   juce::AudioFormatReader &source,
                                   const juce::BigInteger &notes,
                                   int midiNoteForNormalPitch,
                                   double attackTimeSecs,
                                   double releaseTimeSecs,
                                   double maxSampleLengthSeconds)
            : name(soundName),
              sourceSampleRate(source.sampleRate),
              midiNotes(notes),
              midiRootNote(midiNoteForNormalPitch) {
        if (sourceSampleRate > 0 && source.lengthInSamples > 0) {
            length = juce::jmin((int) source.lengthInSamples,
                          (int) (maxSampleLengthSeconds * sourceSampleRate));

            data.reset(new juce::AudioBuffer<float>(juce::jmin(2, (int) source.numChannels), length + 4));

            source.read(data.get(), 0, length + 4, 0, true, true);

            params.attack = static_cast<float> (attackTimeSecs);
            params.release = static_cast<float> (releaseTimeSecs);
        }
    }

    MySamplerSound::~MySamplerSound() {
    }

    bool MySamplerSound::appliesToNote(int midiNoteNumber) {
        return midiNotes[midiNoteNumber];
    }

    bool MySamplerSound::appliesToChannel(int /*midiChannel*/) {
        return true;
    }

//==============================================================================
    MySamplerVoice::MySamplerVoice() {}

    MySamplerVoice::~MySamplerVoice() {}

    bool MySamplerVoice::canPlaySound(juce::SynthesiserSound *sound) {
        return dynamic_cast<const MySamplerSound *> (sound) != nullptr;
    }

    void MySamplerVoice::startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound *s,
                                   int /*currentPitchWheelPosition*/) {
        if (auto *sound = dynamic_cast<const MySamplerSound *> (s)) {
            basePitchRatio = std::pow(2.0, (midiNoteNumber - sound->midiRootNote) / 12.0)
                             * sound->sourceSampleRate / getSampleRate();

            pitchRatio = basePitchRatio * tuneRatio * pitchWheelRatio;

            sourceSamplePosition = 0.0;
            lgain = velocity;
            rgain = velocity;

            adsr.setSampleRate(sound->sourceSampleRate);
            adsr.setParameters(sound->params);

            adsr.noteOn();
        } else {
            jassertfalse; // this object can only play SamplerSounds!
        }
    }

    void MySamplerVoice::stopNote(float /*velocity*/, bool allowTailOff) {
        if (allowTailOff) {
            adsr.noteOff();
        } else {
            clearCurrentNote();
            adsr.reset();
        }
    }

    void MySamplerVoice::pitchWheelMoved(int newValue) {
        float pitchWheelShift = (newValue - 8192) / 4096.0f * 2.0f;
        pitchWheelRatio = std::pow(2.0f, pitchWheelShift / 12.0f);
    }

    void MySamplerVoice::controllerMoved(int /*controllerNumber*/, int /*newValue*/) {}

//==============================================================================
    void MySamplerVoice::renderNextBlock(juce::AudioBuffer<float> &outputBuffer, int startSample, int numSamples) {
        if (auto *playingSound = static_cast<MySamplerSound *> (getCurrentlyPlayingSound().get())) {
            pitchRatio = basePitchRatio * tuneRatio * pitchWheelRatio;
            std::cout << "pitchRatio: " << pitchRatio << std::endl;
            auto &data = *playingSound->data;
            const float *const inL = data.getReadPointer(0);
            const float *const inR = data.getNumChannels() > 1 ? data.getReadPointer(1) : nullptr;

            float *outL = outputBuffer.getWritePointer(0, startSample);
            float *outR = outputBuffer.getNumChannels() > 1 ? outputBuffer.getWritePointer(1, startSample) : nullptr;

            while (--numSamples >= 0) {
                auto pos = (int) sourceSamplePosition;
                auto alpha = (float) (sourceSamplePosition - pos);
                auto invAlpha = 1.0f - alpha;

                // just using a very simple linear interpolation here..
                float l = (inL[pos] * invAlpha + inL[pos + 1] * alpha);
                float r = (inR != nullptr) ? (inR[pos] * invAlpha + inR[pos + 1] * alpha)
                                           : l;

                auto envelopeValue = adsr.getNextSample();

                l *= lgain * envelopeValue;
                r *= rgain * envelopeValue;

                if (outR != nullptr) {
                    *outL++ += l;
                    *outR++ += r;
                } else {
                    *outL++ += (l + r) * 0.5f;
                }

                sourceSamplePosition += pitchRatio;

                if (sourceSamplePosition > playingSound->length) {
                    stopNote(0.0f, false);
                    break;
                }
            }
        }
    }