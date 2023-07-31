//
// Created by Pierre-Louis Suckrow on 30.07.23.
//

#include "SampleHolder.h"

SampleHolder::SampleHolder() {
    std::cout << "hodler_construcotr" << std::endl;

}
SampleHolder::~SampleHolder() {
    std::cout << "holder-destructure" << std::endl;
}

void SampleHolder::setFileName(juce::String fileName){
    fileName_ = fileName;
    try {
        sendChangeMessage();
    } catch (std::exception& e) {
        std::cout << e.what() << std::endl;
    }
}
juce::String SampleHolder::getFileName(){
    return fileName_;
}



juce::AudioThumbnailCache& SampleHolder::getThumbnailCache() {
    return thumbnailCache_;
}

