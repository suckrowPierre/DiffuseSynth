//
// Created by Pierre-Louis Suckrow on 30.07.23.
//

#include "WaveFormDisplay.h"
#include "juce_audio_utils/juce_audio_utils.h"


WaveFormDisplay::WaveFormDisplay() {
    setColour (ColourIDs::waveformBackgroundColour, juce::Colours::transparentBlack);
    setColour (ColourIDs::waveformForegroundColour, juce::Colours::orangered);
}

WaveFormDisplay::~WaveFormDisplay() {
    if (sampleHolder)
        sampleHolder->removeChangeListener (this);
}



void WaveFormDisplay::paint (juce::Graphics& g)
{
    std::cout << "paint" << std::endl;
    auto background = findColour (ColourIDs::waveformBackgroundColour);
    auto foreground = findColour (ColourIDs::waveformForegroundColour);

    if (!background.isTransparent())
    {
        g.fillAll (background);
    }

    g.setColour (foreground);

    if (thumbnail)
    {
        auto peak = thumbnail->getApproximatePeak();
        thumbnail->drawChannels (g, getLocalBounds().reduced (3), 0.0, thumbnail->getTotalLength(), peak > 0.0f ? 1.0f / peak : 1.0f);
    }
    else
        g.drawFittedText (TRANS ("No File"), getLocalBounds(), juce::Justification::centred, 1);
}

void WaveFormDisplay::setAudioThumbnail (SampleHolder* holder)
{
    if (sampleHolder)
        sampleHolder->removeChangeListener (this);

    sampleHolder = holder;

    updateAudioFile();

    if (sampleHolder)
        sampleHolder->addChangeListener (this);
}

void WaveFormDisplay::updateAudioFile()
{
    std::cout << "updateAudioFile" << std::endl;
    if (!sampleHolder)
    {
        thumbnail.reset();
        return;
    }

    if (!sampleHolder->getAudioFile().existsAsFile())
        return;

    std::cout << "updateAudioFile2" << std::endl;
    thumbnail = std::make_unique<juce::AudioThumbnail> (256, sampleHolder->getManager(), sampleHolder->getCache());
    thumbnail->setSource (new juce::FileInputSource (sampleHolder->getAudioFile()));
    thumbnail->addChangeListener (this);
}

void WaveFormDisplay::changeListenerCallback ([[maybe_unused]] juce::ChangeBroadcaster* sender)
{
    if (sender == sampleHolder)
        updateAudioFile();

    repaint();
}

WaveFormDisplayItem::WaveFormDisplayItem (foleys::MagicGUIBuilder& builder, const juce::ValueTree& node) : GuiItem (builder, node)
{
    setColourTranslation ({ { "waveform-background", WaveFormDisplay::ColourIDs::waveformBackgroundColour },
                            { "waveform-colour", WaveFormDisplay::ColourIDs::waveformForegroundColour } });

    addAndMakeVisible (waveFormDisplayItem);
}
WaveFormDisplayItem::~WaveFormDisplayItem() = default;

std::vector<foleys::SettableProperty> WaveFormDisplayItem::getSettableProperties() const
{
std::vector<foleys::SettableProperty> newProperties;
return newProperties;
}

void WaveFormDisplayItem::update()
{

    auto& state          = getMagicState();
    auto* audioThumbnail = state.getObjectWithType<SampleHolder> ("Waveform");

    waveFormDisplayItem.setAudioThumbnail (audioThumbnail);
}


juce::Component* WaveFormDisplayItem::getWrappedComponent()
{
return &waveFormDisplayItem;
}


