//
// Created by Pierre-Louis Suckrow on 30.07.23.
//

#ifndef DIFFUSESYNTH_SAMPLEHOLDER_H
#define DIFFUSESYNTH_SAMPLEHOLDER_H

#include "juce_audio_processors/juce_audio_processors.h"
#include "juce_audio_utils/juce_audio_utils.h"

class SampleHolder: public juce::ChangeBroadcaster {
public:
    SampleHolder();
    ~SampleHolder() override;


   juce::AudioThumbnailCache& getThumbnailCache();
   void setFileName(juce::String fileName);
    juce::String getFileName();
private:
    juce::AudioThumbnailCache thumbnailCache_ = juce::AudioThumbnailCache(1);
    juce::String fileName_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SampleHolder)
};


#endif //DIFFUSESYNTH_SAMPLEHOLDER_H
