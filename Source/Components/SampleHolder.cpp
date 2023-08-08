//
// Created by Pierre-Louis Suckrow on 30.07.23.
// Based on code from Daniel Walz;
//

#include "SampleHolder.h"

SampleHolder::SampleHolder(juce::AudioThumbnailCache& cache, juce::AudioFormatManager& managerToUse) : formatManager (managerToUse), thumbnailCache (cache)
{
}


SampleHolder::~SampleHolder()
{
    masterReference.clear();
}

void SampleHolder::setAudioFile (juce::File file)
{
    audioFile = file;
    sendChangeMessage();
}

juce::File SampleHolder::getAudioFile() const
{
    return audioFile;
}

juce::AudioThumbnailCache& SampleHolder::getCache()
{
    return thumbnailCache;
}

juce::AudioFormatManager& SampleHolder::getManager()
{
    return formatManager;
}
