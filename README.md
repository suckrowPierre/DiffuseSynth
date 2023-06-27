# DiffuseSynth 

A synthesizer using diffusion to generate playable sounds.

## Installation
### VSTs/AUs/Standalone
Download the latest release from the [releases page]()
### Source

#### Mac
Xcode needed for c++ support.

To run the source code clone the repo with `git clone --recurse-submodules` or clone normally and run `git submodule init` and `git submodule update` to get the submodules.

## Usage

write a prompt into field and press generate.

## Credits

This project was build using the [JUCE](https://github.com/juce-framework/JUCE) framework with the Module [foleys_gui_magic](https://github.com/ffAudio/foleys_gui_magic/) for the GUI.

As a diffusion model [AudioLDM](https://github.com/haoheliu/AudioLDM) is being used in the form of a pipeline from [HuggingFace](https://huggingface.co/docs/diffusers/v0.17.1/en/api/pipelines/audioldm)

