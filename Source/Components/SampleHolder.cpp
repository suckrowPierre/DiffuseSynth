//
// Modified by Pierre-Louis Suckrow on 10.08.23.
// Original code created by Daniel Walz on 31.07.23.
//

#include "SampleHolder.h"

namespace foleys
{

// ================================================================================

SampleHolder::SampleHolder (juce::AudioThumbnailCache& cache, juce::AudioFormatManager& managerToUse) : formatManager (managerToUse), thumbnailCache (cache)
{
}

SampleHolder::~SampleHolder()
{
    masterReference.clear();
}

void SampleHolder::setAudioFileAndBuffer (juce::File file, juce::AudioBuffer<float>* buffer)
{
    audioFile = file;
    audioBuffer = buffer;

    sendChangeMessage();
}

juce::AudioBuffer<float>* SampleHolder::getAudioBuffer() const {
    return audioBuffer;
}

juce::File SampleHolder::getAudioFile() const
{
    return audioFile;
}

juce::AudioThumbnailCache& SampleHolder::getCache()
{
    return thumbnailCache;
}

juce::AudioFormatManager& SampleHolder::getManager()
{
    return formatManager;
}

// ================================================================================

WaveformDisplay::WaveformDisplay()
{
    setColour (ColourIds::waveformBackgroundColour, juce::Colours::transparentBlack);
    setColour (ColourIds::waveformForegroundColour, juce::Colours::orangered);
}

WaveformDisplay::~WaveformDisplay()
{
    if (audioThumb)
        audioThumb->removeChangeListener (this);
}

void WaveformDisplay::paint (juce::Graphics& g)
{
    auto background = findColour (ColourIds::waveformBackgroundColour);
    auto foreground = findColour (ColourIds::waveformForegroundColour);

    if (!background.isTransparent())
    {
        g.fillAll (background);
    }

    g.setColour (foreground);

    if (thumbnail)
    {
        auto peak = thumbnail->getApproximatePeak();
        thumbnail->drawChannels (g, getLocalBounds().reduced (3), 0.0, thumbnail->getTotalLength(), peak > 0.0f ? 1.0f / peak : 1.0f);
    }
    else
        g.drawFittedText (TRANS ("No File"), getLocalBounds(), juce::Justification::centred, 1);
}

void WaveformDisplay::setAudioThumbnail (SampleHolder* thumb)
{
    if (audioThumb)
        audioThumb->removeChangeListener (this);

    audioThumb = thumb;

    updateAudioFile();

    if (audioThumb)
        audioThumb->addChangeListener (this);
}

void WaveformDisplay::updateAudioFile()
{
    if (!audioThumb)
    {
        thumbnail.reset();
        return;
    }

    if (!audioThumb->getAudioFile().existsAsFile())
        return;
    audioThumb->getCache().clear();
    thumbnail = std::make_unique<juce::AudioThumbnail> (256, audioThumb->getManager(), audioThumb->getCache());
    thumbnail->setSource (new juce::FileInputSource (audioThumb->getAudioFile()));
    thumbnail->addChangeListener (this);
}

void WaveformDisplay::changeListenerCallback ([[maybe_unused]] juce::ChangeBroadcaster* sender)
{
    if (sender == audioThumb)
        updateAudioFile();

    repaint();
}


// ================================================================================

WaveformItem::WaveformItem (MagicGUIBuilder& builder, const juce::ValueTree& node) : GuiItem (builder, node)
{
    setColourTranslation ({ { "waveform-background", WaveformDisplay::ColourIds::waveformBackgroundColour },
                            { "waveform-colour", WaveformDisplay::ColourIds::waveformForegroundColour } });

    addAndMakeVisible (waveformDisplay);
}

void WaveformItem::update()
{
    auto& state          = getMagicState();
    auto* audioThumbnail = state.getObjectWithType<SampleHolder> ("Waveform");

    waveformDisplay.setAudioThumbnail (audioThumbnail);
}

// ================================================================================

    SpectrogramDisplay::SpectrogramDisplay()
    {
        setColour (ColourIds::spectrogramBackgroundColour, juce::Colours::transparentBlack);
        setColour (ColourIds::spectrogramForegroundColour, juce::Colours::orangered);

        spectrogramGradient.addColour(0.0, juce::Colours::black);
        spectrogramGradient.addColour(0.125, juce::Colour(27,10,66) );
        spectrogramGradient.addColour(0.25, juce::Colour(71,23,116) );
        spectrogramGradient.addColour(0.375, juce::Colour(96,36,100) );
        spectrogramGradient.addColour(0.5, juce::Colour(133,51,96) );
        spectrogramGradient.addColour(0.625, juce::Colour(209,90,100) );
        spectrogramGradient.addColour(0.75, juce::Colour(236,141,106) );
        spectrogramGradient.addColour(0.875, juce::Colour(245,196,143) );
        spectrogramGradient.addColour(1, juce::Colour(253,254,198) );


    }

    SpectrogramDisplay::~SpectrogramDisplay()
    {
        if (audioThumb)
            audioThumb->removeChangeListener (this);
    }

    void SpectrogramDisplay::paint (juce::Graphics& g)
    {
        auto background = findColour (ColourIds::spectrogramBackgroundColour);
        auto foreground = findColour (ColourIds::spectrogramForegroundColour);

        if (!background.isTransparent())
        {
            g.fillAll (background);
        }

        g.setColour (foreground);

        if (audioBuffer != nullptr)
        {
                g.drawImageAt(spectrogramImage, 0, 0);
        }
        else
            g.drawFittedText (TRANS ("No File"), getLocalBounds(), juce::Justification::centred, 1);
    }

    void SpectrogramDisplay::setAudioThumbnail (SampleHolder* thumb)
    {
        if (audioThumb)
            audioThumb->removeChangeListener (this);

        audioThumb = thumb;

        updateAudioFile();

        if (audioThumb)
            audioThumb->addChangeListener (this);
    }

    void SpectrogramDisplay::updateAudioFile()
    {
        if (!audioThumb)
            return;
        if (!audioThumb->getAudioFile().existsAsFile())
            return;

        audioThumb->getCache().clear();
        audioBuffer = audioThumb->getAudioBuffer();
        forwardFFT = std::make_unique<juce::dsp::FFT>(fftOrder);

        updateSpectrogram();
    }

    void SpectrogramDisplay::updateSpectrogram()
    {
        spectrogramImage = juce::Image(juce::Image::RGB, getLocalBounds().getWidth(), getLocalBounds().getHeight(), true);
        currentX = 0;

        const int numChannels = audioBuffer->getNumChannels();
        const int numSamples = audioBuffer->getNumSamples();
        const int spectrogramWidth = getLocalBounds().getWidth();
        const int spectrogramHeight = getLocalBounds().getHeight();

        const int columnsPerFFT = spectrogramWidth / (numSamples / fftSize);

        for (int channel = 0; channel < numChannels; ++channel)
        {
            const float* channelData = audioBuffer->getReadPointer(channel);

            for (int i = 0; i < numSamples; i += fftSize)
            {
                for (int j = 0; j < fftSize; ++j)
                {
                    fftData[j] = (i + j < numSamples) ? channelData[i + j] : 0.f;
                }

                forwardFFT->performFrequencyOnlyForwardTransform(fftData.data());

                auto maxLevel = juce::FloatVectorOperations::findMinAndMax(fftData.data(), fftSize / 2);

                for (int x = 0; x < columnsPerFFT; ++x)
                {
                    for (int y = 0; y < spectrogramHeight; ++y)
                    {
                        auto skewedProportionY = 1.0f - std::exp(std::log(float(y) / float(spectrogramHeight)) * 0.2f);
                        auto fftDataIndex = (size_t)juce::jlimit(0, fftSize / 2, int(skewedProportionY * fftSize / 2));
                        auto level = juce::jmap(fftData[fftDataIndex], 0.0f, juce::jmax(maxLevel.getEnd(), 1e-5f)/3, 0.0f, 1.0f);


                        juce::Colour color = spectrogramGradient.getColourAtPosition(level);
                        spectrogramImage.setPixelAt(currentX, y, color);
                    }

                    currentX++;
                }
            }
        }

        repaint();
    }

    void SpectrogramDisplay::changeListenerCallback ([[maybe_unused]] juce::ChangeBroadcaster* sender)
    {
        if (sender == audioThumb)
            updateAudioFile();

        repaint();
    }
//=================================================================================================


    SpectrogramItem::SpectrogramItem (MagicGUIBuilder& builder, const juce::ValueTree& node) : GuiItem (builder, node)
    {
        setColourTranslation ({ { "spectrogram-background", SpectrogramDisplay::ColourIds::spectrogramBackgroundColour },
                                { "spectrogram-colour", SpectrogramDisplay::ColourIds::spectrogramForegroundColour } });

        addAndMakeVisible (spectrogramDisplay);
    }

    void SpectrogramItem::update()
    {
        auto& state          = getMagicState();
        auto* audioThumbnail = state.getObjectWithType<SampleHolder> ("Waveform");

        spectrogramDisplay.setAudioThumbnail (audioThumbnail);
    }

}  // namespace foleys
