//
// Created by Pierre-Louis Suckrow on 30.07.23.
//

#ifndef DIFFUSESYNTH_WAVEFORMDISPLAY_H
#define DIFFUSESYNTH_WAVEFORMDISPLAY_H

#include <juce_audio_processors/juce_audio_processors.h>
#include "../../Modules/foleys_gui_magic/foleys_gui_magic.h"
#include "../PluginProcessor.h"

class WaveFormDisplay : public juce::Component{
public:
    enum ColourIDs {
        // we are safe from collissions, because we set the colours on every component directly from the stylesheet
        backgroundColourId,
        drawColourId,
        fillColourId
    };

    WaveFormDisplay();
    ~WaveFormDisplay() override;

    void paint(juce::Graphics& g) override;
    void setThumbnail(juce::AudioThumbnail& thumbnail);

private:
    juce::AudioThumbnail* thumbnail_;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveFormDisplay)
};

class WaveFormDisplayItem: public foleys::GuiItem, public juce::ChangeListener {
public:
    FOLEYS_DECLARE_GUI_FACTORY (WaveFormDisplayItem)
    WaveFormDisplayItem (foleys::MagicGUIBuilder& builder, const juce::ValueTree& node);
    ~WaveFormDisplayItem() override;

    std::vector<foleys::SettableProperty> getSettableProperties() const override;
    void update() override;
    juce::Component* getWrappedComponent() override;

private:
    WaveFormDisplay waveFormDisplayItem_;
    SampleHolder* holder;
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WaveFormDisplayItem)


};

#endif //DIFFUSESYNTH_WAVEFORMDISPLAY_H
