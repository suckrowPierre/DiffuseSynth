//
// Created by Pierre-Louis Suckrow on 12.06.23.
//

#ifndef DIFFUSESYNTH_HEADING_H
#define DIFFUSESYNTH_HEADING_H

#include "juce_gui_basics/juce_gui_basics.h"
#include "../LocalizationManager.h"

class Heading: public juce::Component {
public:
    explicit Heading(const int& h);

    ~Heading() override;

    void paint(juce::Graphics &g) override;

    void resized() override;
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Heading)
    int height;
};


#endif //DIFFUSESYNTH_HEADING_H
