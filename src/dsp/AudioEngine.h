#pragma once

#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>
#include "FilterProcessor.h"
#include "CrossfeedProcessor.h"
#include "LimiterProcessor.h"
#include "ConvolutionProcessor.h"
#include "ProfileManager.h"
#include "LoudnessProcessor.h"

namespace equinox
{

/**
 * @class AudioEngine
 * @brief Manages audio devices and signal processing chain.
 */
class AudioEngine : public juce::AudioIODeviceCallback,
                    public juce::ChangeListener
{
public:
    AudioEngine();
    ~AudioEngine() override;

    void initialize();
    void shutdown();

    [[nodiscard]] juce::AudioDeviceManager& getDeviceManager() { return m_deviceManager; }
    [[nodiscard]] FilterProcessor& getFilterProcessor() { return m_deviceFilterProcessor; }
    [[nodiscard]] CrossfeedProcessor& getCrossfeedProcessor() { return m_crossfeedProcessor; }
    [[nodiscard]] LimiterProcessor& getLimiterProcessor() { return m_limiterProcessor; }
    [[nodiscard]] ConvolutionProcessor& getConvolutionProcessor() { return m_convolutionProcessor; }
    [[nodiscard]] LoudnessProcessor& getLoudnessProcessor() { return m_loudnessProcessor; }
    [[nodiscard]] ProfileManager& getProfileManager() { return m_profileManager; }
    
    [[nodiscard]] juce::dsp::Compressor<float>& getCompressor() { return m_compressor; }

    void setConvolutionEnabled(bool enabled) { m_convolutionProcessor.setEnabled(enabled); }
    void setLoudnessEnabled(bool enabled) { m_loudnessProcessor.setEnabled(enabled); }
    void setCompressorEnabled(bool enabled) { m_isCompressorEnabled.store(enabled); }

    void loadImpulseResponse(const juce::File& file) { m_convolutionProcessor.loadImpulseResponse(file); }

    void loadProfile(const juce::String& name);
    void saveCurrentProfileAs(const juce::String& name);
    void mapCurrentDeviceToProfile(const juce::String& profileName);

    [[nodiscard]] juce::AudioPluginFormatManager& getPluginFormatManager() { return m_pluginFormatManager; }
    [[nodiscard]] juce::KnownPluginList& getKnownPluginList() { return m_knownPluginList; }

    void applyAutoPreamp();

    void audioDeviceIOCallbackWithContext(const float* const* inputChannelData,
                                          int numInputChannels,
                                          float* const* outputChannelData,
                                          int numOutputChannels,
                                          int numSamples,
                                          const juce::AudioIODeviceCallbackContext& context) override;

    void audioDeviceAboutToStart(juce::AudioIODevice* device) override;
    void audioDeviceStopped() override;
    void audioDeviceError(const juce::String& configErrorMessage) override;

    // ChangeListener override
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

    void scanPlugins();
    bool addPlugin(const juce::PluginDescription& description);
    void removePlugin(int index);

    [[nodiscard]] int getHostedPluginCount() const { return static_cast<int>(m_pluginNodes.size()); }
    [[nodiscard]] juce::PluginDescription getHostedPluginDescription(int index) const;
    [[nodiscard]] juce::AudioProcessor* getHostedPluginProcessor(int index);

    [[nodiscard]] float getInputLevel() const { return m_inputLevel.load(); }
    [[nodiscard]] float getOutputLevel() const { return m_outputLevel.load(); }

    void setTestToneEnabled(bool shouldBeEnabled) { m_testToneEnabled.store(shouldBeEnabled); }
    [[nodiscard]] bool isTestToneEnabled() const { return m_testToneEnabled.load(); }

    void triggerSampleLog() { m_shouldLogSamples.store(true); }

    // Spectral analysis
    static constexpr int fftOrder = 11;
    static constexpr int fftSize = 1 << fftOrder;
    [[nodiscard]] const float* getFFTData() const { return m_fftData.data(); }

    // Visualizer support
    std::function<void(float)> onSampleReceived;

private:
    void pushNextSampleIntoFifo(float sample) noexcept;

    void setupGraph();
    void updateGraphRouting();

    juce::AudioDeviceManager m_deviceManager;
    ProfileManager m_profileManager;
    FilterProcessor m_deviceFilterProcessor;
    CrossfeedProcessor m_crossfeedProcessor;
    LimiterProcessor m_limiterProcessor;
    ConvolutionProcessor m_convolutionProcessor;
    LoudnessProcessor m_loudnessProcessor;
    
    juce::dsp::Compressor<float> m_compressor;

    std::atomic<bool> m_isCompressorEnabled { false };
    std::atomic<bool> m_shouldLogSamples { false };

    std::atomic<float> m_inputLevel { 0.0f };
    std::atomic<float> m_outputLevel { 0.0f };
    std::atomic<bool> m_testToneEnabled { false };
    float m_testTonePhase = 0.0f;

    std::unique_ptr<juce::AudioProcessorGraph> m_mainGraph;

    juce::AudioPluginFormatManager m_pluginFormatManager;
    juce::KnownPluginList m_knownPluginList;

    juce::AudioProcessorGraph::Node::Ptr m_audioInputNode;
    juce::AudioProcessorGraph::Node::Ptr m_audioOutputNode;
    
    std::vector<juce::AudioProcessorGraph::Node::Ptr> m_pluginNodes;

    // FFT members
    juce::dsp::FFT m_forwardFFT { fftOrder };
    juce::dsp::WindowingFunction<float> m_window { fftSize, juce::dsp::WindowingFunction<float>::hann };
    std::array<float, fftSize> m_fifo;
    std::array<float, fftSize * 2> m_fftData;
    int m_fifoIndex = 0;
    bool m_nextFFTBlockReady = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioEngine)
};

} // namespace equinox
