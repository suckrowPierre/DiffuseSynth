//
// Created by Pierre-Louis Suckrow on 30.07.23.
//

#include "WaveFormDisplay.h"

WaveFormDisplay::WaveFormDisplay() {
    setColour(backgroundColourId, juce::Colours::black);
    setColour(drawColourId, juce::Colours::white);
    setColour(fillColourId, juce::Colours::transparentBlack);
}

WaveFormDisplay::~WaveFormDisplay() {
}

void WaveFormDisplay::paint(juce::Graphics& g) {
g.fillAll(findColour(backgroundColourId));
g.setColour(findColour(drawColourId));
g.drawRect(getLocalBounds(), 1);
g.setColour(findColour(fillColourId));
g.fillRect(getLocalBounds());
}

WaveFormDisplayItem::WaveFormDisplayItem (foleys::MagicGUIBuilder& builder, const juce::ValueTree& node) : foleys::GuiItem (builder, node)
{
setColourTranslation ({
{"testComponent-background", WaveFormDisplay::backgroundColourId},
{"testComponent-draw", WaveFormDisplay::drawColourId},
{"testComponent-fill", WaveFormDisplay::fillColourId} });

addAndMakeVisible (waveFormDisplayItem_);
}

WaveFormDisplayItem::~WaveFormDisplayItem()
{
}

std::vector<foleys::SettableProperty> WaveFormDisplayItem::getSettableProperties() const
{
std::vector<foleys::SettableProperty> newProperties;
return newProperties;
}

void WaveFormDisplayItem::update()
{
}


juce::Component* WaveFormDisplayItem::getWrappedComponent()
{
return &waveFormDisplayItem_;
}