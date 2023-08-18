//
// Created by Pierre-Louis Suckrow on 13.07.23.
//

#ifndef DIFFUSESYNTH_GUIHELPERS_H
#define DIFFUSESYNTH_GUIHELPERS_H

#include <juce_audio_processors/juce_audio_processors.h>
#include <foleys_gui_magic/foleys_gui_magic.h>


class GuiHelpers {
public:
    explicit GuiHelpers (foleys::MagicProcessorState& magicState);

    juce::ValueTree getNodeById(const juce::String& id);
    juce::ValueTree getNodeByPath(const std::vector<int>& path);
    std::vector<int> searchGUITreeRecursively(const juce::ValueTree& node, const juce::Identifier& identifier, const juce::String& id, std::vector<int> path = {});
private:
    foleys::MagicProcessorState& magicState;
    std::map<juce::String, std::vector<int>> nodePaths;
};


#endif //DIFFUSESYNTH_GUIHELPERS_H
