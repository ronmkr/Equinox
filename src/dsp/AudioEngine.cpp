#include "AudioEngine.h"
#include "InternalEqProcessor.h"
#include "LimiterProcessor.h"
#include "CrossfeedProcessor.h"

namespace equinox
{

AudioEngine::AudioEngine()
{
    m_mainGraph = std::make_unique<juce::AudioProcessorGraph>();
    
    // Register standard plugin formats
    juce::addDefaultFormatsToManager (m_pluginFormatManager);
}

AudioEngine::~AudioEngine()
{
    shutdown();
}

void AudioEngine::initialize()
{
    auto error = m_deviceManager.initialise(2, 2, nullptr, true);
    
    if (error.isNotEmpty()) {
        juce::Logger::writeToLog("AudioEngine Error: " + error);
    }

    setupGraph();
    
    m_graphPlayer.setProcessor(m_mainGraph.get());
    m_deviceManager.addAudioCallback(&m_graphPlayer);
    m_deviceManager.addAudioCallback(this);
}

void AudioEngine::shutdown()
{
    m_deviceManager.removeAudioCallback(&m_graphPlayer);
    m_deviceManager.removeAudioCallback(this);
    
    m_graphPlayer.setProcessor(nullptr);
    m_mainGraph->clear();
    
    m_deviceManager.closeAudioDevice();
}

void AudioEngine::setupGraph()
{
    m_mainGraph->clear();

    m_audioInputNode = m_mainGraph->addNode(std::make_unique<juce::AudioProcessorGraph::AudioGraphIOProcessor>(
        juce::AudioProcessorGraph::AudioGraphIOProcessor::audioInputNode));
    
    m_audioOutputNode = m_mainGraph->addNode(std::make_unique<juce::AudioProcessorGraph::AudioGraphIOProcessor>(
        juce::AudioProcessorGraph::AudioGraphIOProcessor::audioOutputNode));

    m_eqNode = m_mainGraph->addNode(std::make_unique<InternalEqProcessor>(m_deviceFilterProcessor));
    m_crossfeedNode = m_mainGraph->addNode(std::make_unique<CrossfeedProcessor>());
    m_limiterNode = m_mainGraph->addNode(std::make_unique<LimiterProcessor>());

    updateGraphRouting();
}

void AudioEngine::scanPlugins()
{
    juce::FileSearchPath v3Path("/Library/Audio/Plug-Ins/VST3");
    juce::FileSearchPath auPath("/Library/Audio/Plug-Ins/Components");

    for (auto* format : m_pluginFormatManager.getFormats())
    {
        if (format->getName() == "VST3")
        {
            auto scanner = std::make_unique<juce::PluginDirectoryScanner>(m_knownPluginList, *format, v3Path, true, juce::File());
            juce::String name;
            while (scanner->scanNextFile(true, name)) {}
        }
        else if (format->getName() == "AudioUnit")
        {
            auto scanner = std::make_unique<juce::PluginDirectoryScanner>(m_knownPluginList, *format, auPath, true, juce::File());
            juce::String name;
            while (scanner->scanNextFile(true, name)) {}
        }
    }
}

bool AudioEngine::addPlugin(const juce::PluginDescription& description)
{
    juce::String errorMessage;
    auto instance = m_pluginFormatManager.createPluginInstance(description, 
                                                              m_mainGraph->getSampleRate(), 
                                                              m_mainGraph->getBlockSize(), 
                                                              errorMessage);
    if (instance == nullptr)
    {
        juce::Logger::writeToLog("Error loading plugin: " + errorMessage);
        return false;
    }

    auto node = m_mainGraph->addNode(std::move(instance));
    if (node)
    {
        m_pluginNodes.push_back(node);
        updateGraphRouting();
        return true;
    }

    return false;
}

void AudioEngine::removePlugin(int index)
{
    if (index >= 0 && index < (int)m_pluginNodes.size())
    {
        m_mainGraph->removeNode(m_pluginNodes[index].get());
        m_pluginNodes.erase(m_pluginNodes.begin() + index);
        updateGraphRouting();
    }
}

void AudioEngine::applyAutoPreamp()
{
    float maxGain = m_deviceFilterProcessor.calculateMaxGain();
    // Set preamp to -maxGain with a small 0.1dB safety buffer
    m_deviceFilterProcessor.setPreamp(-(maxGain + 0.1f));
}

void AudioEngine::updateGraphRouting()
{
    if (m_audioInputNode == nullptr || m_audioOutputNode == nullptr || 
        m_eqNode == nullptr || m_crossfeedNode == nullptr || m_limiterNode == nullptr)
        return;

    m_mainGraph->suspendProcessing(true);

    for (const auto& connection : m_mainGraph->getConnections())
        m_mainGraph->removeConnection(connection);

    // Current Chain: Input -> EQ -> Crossfeed -> [Plugins] -> Limiter -> Output
    
    auto lastNode = m_eqNode;

    // Connect Input -> EQ
    for (int channel = 0; channel < 2; ++channel)
    {
        m_mainGraph->addConnection({ { m_audioInputNode->nodeID, channel },
                                     { m_eqNode->nodeID, channel } });
    }

    // Connect EQ -> Crossfeed
    for (int channel = 0; channel < 2; ++channel)
    {
        m_mainGraph->addConnection({ { m_eqNode->nodeID, channel },
                                     { m_crossfeedNode->nodeID, channel } });
    }
    
    lastNode = m_crossfeedNode;

    // Connect Plugins in series
    for (auto& pluginNode : m_pluginNodes)
    {
        for (int channel = 0; channel < 2; ++channel)
        {
            m_mainGraph->addConnection({ { lastNode->nodeID, channel },
                                         { pluginNode->nodeID, channel } });
        }
        lastNode = pluginNode;
    }

    // Connect last node to Limiter
    for (int channel = 0; channel < 2; ++channel)
    {
        m_mainGraph->addConnection({ { lastNode->nodeID, channel },
                                     { m_limiterNode->nodeID, channel } });
    }

    // Connect Limiter to Output
    for (int channel = 0; channel < 2; ++channel)
    {
        m_mainGraph->addConnection({ { m_limiterNode->nodeID, channel },
                                     { m_audioOutputNode->nodeID, channel } });
    }

    m_mainGraph->suspendProcessing(false);
}

void AudioEngine::audioDeviceAboutToStart(juce::AudioIODevice* )
{
    updateGraphRouting();
}

void AudioEngine::audioDeviceStopped() {}

void AudioEngine::audioDeviceIOCallbackWithContext(const float* const* ,
                                                  int ,
                                                  float* const* ,
                                                  int ,
                                                  int ,
                                                  const juce::AudioIODeviceCallbackContext& )
{}

void AudioEngine::audioDeviceError(const juce::String& configErrorMessage)
{
    juce::Logger::writeToLog("Audio Device Error: " + configErrorMessage);
}

} // namespace equinox
