//
// Created by Pierre-Louis Suckrow on 13.07.23.
//

#include "GuiHelpers.h"

GuiHelpers::GuiHelpers(foleys::MagicProcessorState& magicState) : magicState(magicState) {

}


juce::ValueTree GuiHelpers::getNodeById(const juce::String& id) {
    if(nodePaths.count(id) == 0) // If the path for this id is not stored
    {
        juce::Identifier identifier = juce::Identifier("id");
        auto guiTree = magicState.getGuiTree().getChild(1);
        nodePaths[id] = searchGUITreeRecursively(guiTree, identifier, id); // Store the path
    }

    return getNodeByPath(nodePaths[id]); // Retrieve the node by the stored path
}

juce::ValueTree GuiHelpers::getNodeByPath(const std::vector<int>& path) {
    auto node = magicState.getGuiTree().getChild(1); // Start from the initial node
    for(auto index : path)
    {
        if(node.getNumChildren() > index)
            node = node.getChild(index);
        else
            return {}; // Return an invalid ValueTree if the path does not exist
    }

    return node;
}

std::vector<int> GuiHelpers::searchGUITreeRecursively(const juce::ValueTree& node, const juce::Identifier& identifier, const juce::String& id, std::vector<int> path) {
    int numberChildren = node.getNumChildren();
    for (int i = 0; i < numberChildren; i++) {
        auto child = node.getChild(i);
        if (child.isValid() && child.hasProperty(identifier) && child.getProperty(identifier) == id) {
            path.push_back(i);
            return path;
        } else {
            auto newPath = path;
            newPath.push_back(i);
            auto result = searchGUITreeRecursively(child, identifier, id, newPath);
            if (!result.empty()) {
                return result;
            }
        }
    }

    return {};
}