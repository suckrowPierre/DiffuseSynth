//
// Created by Pierre-Louis Suckrow on 12.06.23.
//

#include "Heading.h"
#include "../LocalizationManager.h"


extern LocalizationManager* _manager;




Heading::Heading(const int& h): height(h) {
}
Heading::~Heading() {
}

void Heading::paint(juce::Graphics &g) {

    juce::Rectangle <int> title(0, 0, getParentWidth()/3, height) ;

    //g.setColour(juce::Colours::black);
    //g.fillCheckerBoard(wall, 10, 10, juce::Colours::green, juce::Colours::darkgreen);
    //g.drawRect(title);
    //display text
    g.setColour(juce::Colours::white);
    g.setFont(35.0f);
    juce::String titleText = _manager->getLocalizedString("synth.title");
    g.drawText(titleText, title, juce::Justification::centred, true);
}

void Heading::resized() {}