#pragma once

#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "FilterProcessor.h"

namespace equinox
{

/**
 * @class AudioEngine
 * @brief Manages audio devices and the signal routing from input to output using an AudioProcessorGraph.
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
    [[nodiscard]] juce::AudioPluginFormatManager& getPluginFormatManager() { return m_pluginFormatManager; }
    [[nodiscard]] juce::KnownPluginList& getKnownPluginList() { return m_knownPluginList; }

    [[nodiscard]] juce::AudioProcessorGraph::Node::Ptr getCrossfeedNode() { return m_crossfeedNode; }

    /**
     * @brief Automatically adjusts the preamp gain to avoid clipping.
     */
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

    [[nodiscard]] juce::PluginDescription getHostedPluginDescription(int index) const
    {
        if (auto* instance = dynamic_cast<juce::AudioPluginInstance*>(m_pluginNodes[index]->getProcessor()))
            return instance->getPluginDescription();
        
        return {};
    }

    [[nodiscard]] juce::AudioProcessor* getHostedPluginProcessor(int index)
    {
        return m_pluginNodes[index]->getProcessor();
    }

private:
    void setupGraph();
    void updateGraphRouting();

    juce::AudioDeviceManager m_deviceManager;
    FilterProcessor m_deviceFilterProcessor; // Renamed from filterProcessor to avoid confusion with m_filterProcessor
    
    std::unique_ptr<juce::AudioProcessorGraph> m_mainGraph;
    juce::AudioProcessorPlayer m_graphPlayer;

    juce::AudioPluginFormatManager m_pluginFormatManager;
    juce::KnownPluginList m_knownPluginList;
    std::unique_ptr<juce::PluginDirectoryScanner> m_pluginScanner;

    juce::AudioProcessorGraph::Node::Ptr m_audioInputNode;
    juce::AudioProcessorGraph::Node::Ptr m_audioOutputNode;
    juce::AudioProcessorGraph::Node::Ptr m_eqNode;
    juce::AudioProcessorGraph::Node::Ptr m_crossfeedNode;
    juce::AudioProcessorGraph::Node::Ptr m_limiterNode;
    
    std::vector<juce::AudioProcessorGraph::Node::Ptr> m_pluginNodes;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioEngine)
};

} // namespace equinox
