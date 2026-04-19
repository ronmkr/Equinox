#pragma once

#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>
#include "FilterProcessor.h"
#include "CrossfeedProcessor.h"
#include "LimiterProcessor.h"

namespace equinox
{

/**
 * @class AudioEngine
 * @brief Manages audio devices and signal processing chain.
 */
class AudioEngine : public juce::AudioIODeviceCallback
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
    
    [[nodiscard]] juce::dsp::Convolution& getConvolver() { return m_convolver; }
    [[nodiscard]] juce::dsp::Compressor<float>& getCompressor() { return m_compressor; }

    void setConvolutionEnabled(bool enabled) { m_isConvolutionEnabled.store(enabled); }
    void setCompressorEnabled(bool enabled) { m_isCompressorEnabled.store(enabled); }

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

    // Visualizer support
    std::function<void(float)> onSampleReceived;

private:
    void setupGraph();
    void updateGraphRouting();

    juce::AudioDeviceManager m_deviceManager;
    FilterProcessor m_deviceFilterProcessor;
    CrossfeedProcessor m_crossfeedProcessor;
    LimiterProcessor m_limiterProcessor;
    
    juce::dsp::Convolution m_convolver { juce::dsp::Convolution::Latency { 0 } };
    juce::dsp::Compressor<float> m_compressor;

    std::atomic<bool> m_isConvolutionEnabled { false };
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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioEngine)
};

} // namespace equinox
