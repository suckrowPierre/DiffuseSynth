//
// Created by Pierre-Louis Suckrow on 30.07.23.
//

#include "WaveFormDisplay.h"
#include "juce_audio_utils/juce_audio_utils.h"


WaveFormDisplay::WaveFormDisplay() {
    setColour(backgroundColourId, juce::Colours::black);
    setColour(drawColourId, juce::Colours::white);
    setColour(fillColourId, juce::Colours::transparentBlack);
}

WaveFormDisplay::~WaveFormDisplay() {
}

void WaveFormDisplay::setThumbnail(juce::AudioThumbnail& thumbnail){
    thumbnail_ = &thumbnail;
}


void WaveFormDisplay::paint(juce::Graphics& g) {
g.fillAll(findColour(backgroundColourId));
    juce::Rectangle<int> thumbnailBounds = getLocalBounds();

    g.setColour (juce::Colours::red);                               // [8]

    g.setColour (juce::Colours::darkgrey);
    g.fillRect (thumbnailBounds);
    g.setColour (juce::Colours::white);
    g.drawFittedText ("no sample generated", thumbnailBounds, juce::Justification::centred, 1);
}

WaveFormDisplayItem::WaveFormDisplayItem (foleys::MagicGUIBuilder& builder, const juce::ValueTree& node) : foleys::GuiItem (builder, node)
{
setColourTranslation ({
{"testComponent-background", WaveFormDisplay::backgroundColourId},
{"testComponent-draw", WaveFormDisplay::drawColourId},
{"testComponent-fill", WaveFormDisplay::fillColourId} });

addAndMakeVisible (waveFormDisplayItem_);
    holder = getMagicState().getObjectWithType<SampleHolder>("sample");
    if (holder == nullptr) {
       std::cout << "holder is null" << std::endl;
    } else {
        holder->addChangeListener(this);
    }
}

WaveFormDisplayItem::~WaveFormDisplayItem()
{
    holder->removeChangeListener(this);
}

std::vector<foleys::SettableProperty> WaveFormDisplayItem::getSettableProperties() const
{
std::vector<foleys::SettableProperty> newProperties;
return newProperties;
}

void WaveFormDisplayItem::update()
{

    std::cout << "update" << std::endl;
    juce::String fileName = "";
    try {
        fileName = holder->getFileName();
        std::cout << fileName << std::endl;
    } catch (std::exception& e) {
        std::cout << e.what() << std::endl;
    }
    if(fileName == ""){
        return;
    }

    juce::AudioFormatManager formatManager;
    formatManager.registerBasicFormats();
    auto thumbnail = juce::AudioThumbnail(512, formatManager,  holder->getThumbnailCache());
    thumbnail.setSource(new juce::FileInputSource(holder->getFileName()));
    std::cout << thumbnail.getTotalLength() << std::endl;
}

void WaveFormDisplayItem::changeListenerCallback(juce::ChangeBroadcaster* source) {
    if (source == holder && holder != nullptr) {
        update();
    }

}


juce::Component* WaveFormDisplayItem::getWrappedComponent()
{
return &waveFormDisplayItem_;
}