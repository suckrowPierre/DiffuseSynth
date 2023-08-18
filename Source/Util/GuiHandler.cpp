//
// Created by Pierre-Louis Suckrow on 12.07.23.
//

#include "GuiHandler.h"
#include "../PluginProcessor.h"
#include "juce_gui_basics/juce_gui_basics.h"

GuiHandler::GuiHandler(AudioPluginAudioProcessor& processor, foleys::MagicProcessorState& magicState)
        : processor_(processor), magicState_(magicState), guiHelpers_(std::make_unique<GuiHelpers>(magicState))
{}

void GuiHandler::updateGUIStatus(const juce::String& id, const juce::String& colour) {
    auto node = getNodeByIdAndLogIfInvalid(id);
    if (!node.isValid())
        return;

    if(node.isValid()) {
        node.setProperty(juce::Identifier("label-text"), colour, nullptr);
    }
}

juce::ValueTree GuiHandler::getNodeByIdAndLogIfInvalid(const juce::String& nodeId) {
    auto node = guiHelpers_->getNodeById(nodeId);

    if (!node.isValid()) {
        juce::Logger::writeToLog("Failed to find node with id: " + nodeId);
    }

    return node;
}














