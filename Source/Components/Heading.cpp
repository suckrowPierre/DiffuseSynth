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

    int titleWidth = getParentWidth()/4;
    juce::Rectangle <int> title(0, 0, titleWidth, height) ;
    int subtitleWidth = getParentWidth()-titleWidth-150;
    juce::Rectangle <int> subtitle(titleWidth, 0, subtitleWidth, height) ;
    juce::Line <float> headingLine(juce::Point<float>(0, static_cast<float>(height)), juce::Point<float>(
            static_cast<float>(getParentWidth()), static_cast<float>(height)));


    //g.setColour(juce::Colours::black);
    //g.fillCheckerBoard(wall, 10, 10, juce::Colours::green, juce::Colours::darkgreen);
    //g.drawRect(title);
    //g.drawRect(subtitle);
    g.setColour(juce::Colours::white);
    g.drawLine(headingLine,2.0f);

    //display text


    //title
    g.setFont(35.0f);
    juce::String titleText = _manager->getLocalizedString("synth.title");
    g.drawText(titleText, title, juce::Justification::centred, true);

    //subtitle
    g.setFont(25.0f);
    juce::String subtitleText = _manager->getLocalizedString("synth.subtitle");
    g.drawText(subtitleText, subtitle, juce::Justification::left, true);
}

void Heading::resized() {}