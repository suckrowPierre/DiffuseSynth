//
// Created by Daniel Walz on 31.07.23.
//

#pragma once

#include <foleys_gui_magic/foleys_gui_magic.h>



namespace foleys
{

// ================================================================================

/*!
 * @class AudioThumbnail
 * @brief An AudioThumbnail backend to be displayed in the GUI
 *
 * This serves as backend for an audio file to be displayed in the GUI.
 * The WaveformDisplay will react to a change of the file.
 */
class SampleHolder : public juce::ChangeBroadcaster
{
public:
    SampleHolder (juce::AudioThumbnailCache& cache, juce::AudioFormatManager& manager);
    ~SampleHolder() override;

    /*!
     * Set the audiofile to display
     * @param file the audiofile
     */
    void setAudioFileAndBuffer (juce::File file, juce::AudioBuffer<float>* buffer);

    /*!
     * @return the currently displayed audio file
     */
    juce::File getAudioFile() const;
    juce::AudioBuffer<float>* getAudioBuffer() const;

    juce::AudioThumbnailCache& getCache();
    juce::AudioFormatManager&  getManager();

private:
    juce::AudioFormatManager&  formatManager;
    juce::AudioThumbnailCache& thumbnailCache;
    juce::File                 audioFile;
    juce::AudioBuffer<float>* audioBuffer = nullptr;

    JUCE_DECLARE_WEAK_REFERENCEABLE (SampleHolder)
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SampleHolder)
};

// ================================================================================

/*!
 * @class WaveformDisplay
 * @brief This widget displays an audio file using juce::AudioThumbnail
 */
class WaveformDisplay
  : public juce::Component
  , juce::ChangeListener
{
public:
    enum ColourIds
    {
        waveformBackgroundColour = 0x2002020,
        waveformForegroundColour
    };

    WaveformDisplay();
    ~WaveformDisplay() override;

    void paint (juce::Graphics& g) override;

    void setAudioThumbnail (SampleHolder* thumb);
    void updateAudioFile();

    void changeListenerCallback ([[maybe_unused]] juce::ChangeBroadcaster* sender) override;

private:
    SampleHolder*                       audioThumb = nullptr;
    std::unique_ptr<juce::AudioThumbnail> thumbnail;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WaveformDisplay)
};

// ================================================================================

/*!
 * @class WaveformItem
 * @brief This is a wrapper class so the WaveformDisplay can be used in GuiMagic
 */
class WaveformItem : public GuiItem
{
public:
    WaveformItem (MagicGUIBuilder& builder, const juce::ValueTree& node);

    void             update() override;
    juce::Component* getWrappedComponent() override { return &waveformDisplay; }
    FOLEYS_DECLARE_GUI_FACTORY (WaveformItem)
private:
    WaveformDisplay waveformDisplay;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WaveformItem)
};

// ================================================================================
class SpectrogramDisplay
  : public juce::Component
  , juce::ChangeListener {
    public:
    enum ColourIds {
        spectrogramBackgroundColour = 0x2002020,
        spectrogramForegroundColour
    };

    SpectrogramDisplay();
    ~SpectrogramDisplay() override;

    void paint(juce::Graphics& g) override;

    void setAudioThumbnail (SampleHolder* thumb);
    void updateAudioFile();
    void updateSpectrogram();

    void changeListenerCallback([[maybe_unused]] juce::ChangeBroadcaster* sender) override;

private:
    SampleHolder*                       audioThumb = nullptr;
    juce::AudioBuffer<float>* audioBuffer; // Buffer to hold audio data

    static constexpr auto fftOrder = 10;
    static constexpr auto fftSize  = 1 << fftOrder;
    int currentX = 0;

    std::array<float, fftSize> fifo;
    std::array<float, fftSize * 2> fftData;


    std::unique_ptr<juce::dsp::FFT> forwardFFT;       // FFT object to perform the transform
    juce::Image spectrogramImage;         // Image to draw the spectrogram

    juce::ColourGradient spectrogramGradient;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpectrogramDisplay)
};


class SpectrogramItem : public GuiItem {
public:
    SpectrogramItem(MagicGUIBuilder &builder, const juce::ValueTree &node);

    void update() override;

    juce::Component *getWrappedComponent() override { return &spectrogramDisplay; }
    FOLEYS_DECLARE_GUI_FACTORY (SpectrogramItem)

private:
    SpectrogramDisplay spectrogramDisplay;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpectrogramItem)

};

}  // namespace foleys
