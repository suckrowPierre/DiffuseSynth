# WaveGenSynth
<img width="1197" alt="synth" src="https://github.com/suckrowPierre/WaveGenSynth/assets/100494266/cd185e80-8fc1-48cf-a2e1-42aea68deaf6">

WaveGenSynth is a synthesizer that utilizes diffusion techniques to generate playable sounds. It can be integrated into your workflow as a VST, AU, standalone application, or even run from the source.

## Table of Contents
- [Installation](#installation)
  - [VSTs/AUs/Standalone](#vsts/auss/standalone)
  - [Source](#source)
    - [Mac](#mac)
- [Usage](#usage)
- [Credits](#credits)

## Installation

### VSTs/AUs/Standalone
Download the latest stable release from the [releases page](), and follow the instructions provided for your specific platform.

Installer and Server are availabe for Intel and Silicone Mac

### Source
If you prefer to run the source code, follow these steps:

1. **Clone the Repository**:
   - You can clone the repository and its submodules using the following command: `git clone --recurse-submodules <REPOSITORY_URL>`
   - Or, if cloning normally, initialize and update the submodules with:
     - `git submodule init`
     - `git submodule update`

3. **Load the Project**: Open the project via the provided CMake file.

4. **Configure Run/Debug Settings**: Create a Run/Debug Configuration by choosing one of the executables to run the project.

5. **Set Up the Server** (without the plugin):
   - Update the environment: `conda env update --file environment.yml` (in the FastAPI folder)
   - Activate the environment: `conda activate WaveGenSynthAPI_env`
   - Start the server: `uvicorn server:app --reload`

#### Mac
If you are on a Mac, you will need Xcode for C++ support.

#### Windows
Not yet tested

## Usage

Using DiffuseSynth is straightforward:
1. Start Server executable and plugin.
2. Init a model. May take a while and freezes the plugin. ! (Choose cuda as a device if running on hardware with a GPU, mps if running on Apple Silicone, and cpu as a fallback)
3. Write a prompt into the designated field.
4. Press the "generate" button.
6. Select the appropriate MIDI input device.
7. Play Sound

Explore, experiment, and enjoy the sounds you create!

## Credits

DiffuseSynth was built using these technologies:
- **GUI Framework**: [JUCE](https://github.com/juce-framework/JUCE)
- **GUI Module**: [foleys_gui_magic](https://github.com/ffAudio/foleys_gui_magic/)
- **Diffusion Model**: [AudioLDM](https://github.com/haoheliu/AudioLDM), in the form of a pipeline from [HuggingFace](https://huggingface.co/docs/diffusers/v0.17.1/en/api/pipelines/audioldm)

I extend my gratitude to the developers of these tools that made DiffuseSynth possible.
