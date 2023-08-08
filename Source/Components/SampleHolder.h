//
// Created by Pierre-Louis Suckrow on 30.07.23.
//

#ifndef DIFFUSESYNTH_SAMPLEHOLDER_H
#define DIFFUSESYNTH_SAMPLEHOLDER_H

#include "juce_audio_processors/juce_audio_processors.h"
#include "juce_audio_utils/juce_audio_utils.h"

class SampleHolder: public juce::ChangeBroadcaster {
public:
    SampleHolder(juce::AudioThumbnailCache& cache, juce::AudioFormatManager& manager);
    ~SampleHolder() override;


    void setAudioFile (juce::File file);
    juce::File getAudioFile() const;

    juce::AudioThumbnailCache& getCache();
    juce::AudioFormatManager&  getManager();

private:
    juce::AudioFormatManager& formatManager;
    juce::AudioThumbnailCache& thumbnailCache;
    juce::File audioFile;


    JUCE_DECLARE_WEAK_REFERENCEABLE (SampleHolder)
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SampleHolder)
};


#endif //DIFFUSESYNTH_SAMPLEHOLDER_H
