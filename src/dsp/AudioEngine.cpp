#include "AudioEngine.h"

namespace equinox
{

AudioEngine::AudioEngine()
{
    m_mainGraph = std::make_unique<juce::AudioProcessorGraph>();
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
    m_deviceManager.addAudioCallback(this);
    m_deviceManager.addChangeListener(this);
}

void AudioEngine::shutdown()
{
    m_deviceManager.removeChangeListener(this);
    m_deviceManager.removeAudioCallback(this);
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

    updateGraphRouting();
}

void AudioEngine::updateGraphRouting()
{
    if (m_audioInputNode == nullptr || m_audioOutputNode == nullptr)
        return;

    m_mainGraph->suspendProcessing(true);
    for (const auto& connection : m_mainGraph->getConnections())
        m_mainGraph->removeConnection(connection);

    auto lastNode = m_audioInputNode;

    for (auto& pluginNode : m_pluginNodes) {
        for (int ch = 0; ch < 2; ++ch)
            m_mainGraph->addConnection({ { lastNode->nodeID, ch }, { pluginNode->nodeID, ch } });
        lastNode = pluginNode;
    }

    for (int ch = 0; ch < 2; ++ch)
        m_mainGraph->addConnection({ { lastNode->nodeID, ch }, { m_audioOutputNode->nodeID, ch } });

    m_mainGraph->suspendProcessing(false);
}

void AudioEngine::pushNextSampleIntoFifo(float sample) noexcept
{
    if (m_fifoIndex == fftSize)
    {
        if (!m_nextFFTBlockReady)
        {
            std::fill(m_fftData.begin(), m_fftData.end(), 0.0f);
            std::copy(m_fifo.begin(), m_fifo.end(), m_fftData.begin());
            m_nextFFTBlockReady = true;

            m_window.multiplyWithWindowingTable(m_fftData.data(), fftSize);
            m_forwardFFT.performFrequencyOnlyForwardTransform(m_fftData.data());
            
            m_nextFFTBlockReady = false;
        }
        m_fifoIndex = 0;
    }
    m_fifo[m_fifoIndex++] = sample;
}

void AudioEngine::audioDeviceAboutToStart(juce::AudioIODevice* device)
{
    if (device)
    {
        auto rate = device->getCurrentSampleRate();
        auto buffer = device->getCurrentBufferSizeSamples();
        auto numIns = device->getActiveInputChannels().countNumberOfSetBits();
        auto numOuts = device->getActiveOutputChannels().countNumberOfSetBits();
        
        m_deviceFilterProcessor.prepareToPlay(rate, buffer, std::max(numIns, numOuts));
        m_crossfeedProcessor.prepareToPlay(rate, buffer);
        m_limiterProcessor.prepareToPlay(rate, buffer);
        m_convolutionProcessor.prepareToPlay(rate, buffer);
        m_loudnessProcessor.prepareToPlay(rate, buffer);
        
        juce::dsp::ProcessSpec spec { rate, (juce::uint32)buffer, (juce::uint32)numOuts };
        m_compressor.prepare(spec);

        m_mainGraph->setPlayConfigDetails(numIns, numOuts, rate, buffer);
        m_mainGraph->prepareToPlay(rate, buffer);
    }
}

void AudioEngine::audioDeviceStopped() {}

void AudioEngine::audioDeviceIOCallbackWithContext(const float* const* inputChannelData,
                                                  int numInputChannels,
                                                  float* const* outputChannelData,
                                                  int numOutputChannels,
                                                  int numSamples,
                                                  const juce::AudioIODeviceCallbackContext& )
{
    if (numOutputChannels == 0) return;

    juce::AudioBuffer<float> buffer(outputChannelData, numOutputChannels, numSamples);
    
    float inputMag = 0.0f;
    for (int i = 0; i < numOutputChannels; ++i)
    {
        if (i < numInputChannels && inputChannelData[i] != nullptr)
        {
            buffer.copyFrom(i, 0, inputChannelData[i], numSamples);
            inputMag = std::max(inputMag, buffer.getMagnitude(i, 0, numSamples));
        }
        else
            buffer.clear(i, 0, numSamples);
    }
    m_inputLevel.store(inputMag);

    if (inputMag > 0.000001f || m_testToneEnabled.load())
    {
        m_deviceFilterProcessor.processBlock(buffer);

        juce::MidiBuffer midi;
        if (buffer.getNumChannels() >= 2) {
            m_crossfeedProcessor.processBlock(buffer, midi);
        }

        m_loudnessProcessor.setVolumeDb(m_deviceFilterProcessor.getPreamp());
        m_loudnessProcessor.processBlock(buffer, midi);
        m_convolutionProcessor.processBlock(buffer, midi);

        if (m_isCompressorEnabled.load()) {
            juce::dsp::AudioBlock<float> block(buffer);
            juce::dsp::ProcessContextReplacing<float> context(block);
            m_compressor.process(context);
        }

        m_mainGraph->processBlock(buffer, midi);
        m_limiterProcessor.processBlock(buffer, midi);

        // Feed FFT
        auto* chL = buffer.getReadPointer(0);
        for (int i = 0; i < numSamples; ++i)
            pushNextSampleIntoFifo(chL[i]);
    }

    float outMag = buffer.getMagnitude(0, buffer.getNumSamples());
    m_outputLevel.store(outMag);
}

void AudioEngine::audioDeviceError(const juce::String& configErrorMessage)
{
    juce::Logger::writeToLog("Audio Device Error: " + configErrorMessage);
}

void AudioEngine::loadProfile(const juce::String& name)
{
    auto profile = m_profileManager.loadProfile(name);
    if (profile.name == name)
    {
        m_deviceFilterProcessor.setPreamp(profile.preampDb);
        const auto isoFrequencies = { 20.0f, 25.0f, 31.5f, 40.0f, 50.0f, 63.0f, 80.0f, 100.0f, 125.0f, 160.0f, 
                                     200.0f, 250.0f, 315.0f, 400.0f, 500.0f, 630.0f, 800.0f, 1000.0f, 1250.0f, 1600.0f, 
                                     2000.0f, 2500.0f, 3150.0f, 4000.0f, 5000.0f, 6300.0f, 8000.0f, 10000.0f, 12500.0f, 16000.0f, 20000.0f };
        
        int i = 0;
        for (auto f : isoFrequencies)
        {
            if (i < (int)profile.gains.size())
                m_deviceFilterProcessor.updateBandGain(i, f, profile.gains[i], 1.0f);
            i++;
        }
    }
}

void AudioEngine::saveCurrentProfileAs(const juce::String& name)
{
    EqProfile profile;
    profile.name = name;
    profile.preampDb = m_deviceFilterProcessor.getPreamp();
    profile.gains = m_deviceFilterProcessor.getGains();
    m_profileManager.saveProfile(profile);
}

void AudioEngine::mapCurrentDeviceToProfile(const juce::String& profileName)
{
    if (auto* output = m_deviceManager.getCurrentAudioDevice())
    {
        m_profileManager.setProfileForDevice(output->getName(), profileName);
    }
}

void AudioEngine::changeListenerCallback(juce::ChangeBroadcaster*)
{
    if (auto* output = m_deviceManager.getCurrentAudioDevice())
    {
        auto profileName = m_profileManager.getProfileForDevice(output->getName());
        if (profileName.isNotEmpty())
        {
            loadProfile(profileName);
            juce::Logger::writeToLog("Equinox: Auto-loaded profile '" + profileName + "' for device '" + output->getName() + "'");
        }
    }
}

void AudioEngine::scanPlugins()
{
    juce::FileSearchPath v3Path("/Library/Audio/Plug-Ins/VST3");
    juce::FileSearchPath auPath("/Library/Audio/Plug-Ins/Components");
    for (auto* format : m_pluginFormatManager.getFormats())
    {
        auto path = (format->getName() == "VST3") ? v3Path : auPath;
        auto scanner = std::make_unique<juce::PluginDirectoryScanner>(m_knownPluginList, *format, path, true, juce::File());
        juce::String name;
        while (scanner->scanNextFile(true, name)) {}
    }
}

bool AudioEngine::addPlugin(const juce::PluginDescription& description)
{
    juce::String errorMessage;
    auto instance = m_pluginFormatManager.createPluginInstance(description, 
                                                              m_mainGraph->getSampleRate(), 
                                                              m_mainGraph->getBlockSize(), 
                                                              errorMessage);
    if (instance == nullptr) return false;
    auto node = m_mainGraph->addNode(std::move(instance));
    if (node) {
        m_pluginNodes.push_back(node);
        updateGraphRouting();
        return true;
    }
    return false;
}

void AudioEngine::removePlugin(int index)
{
    if (index >= 0 && index < (int)m_pluginNodes.size()) {
        m_mainGraph->removeNode(m_pluginNodes[index].get());
        m_pluginNodes.erase(m_pluginNodes.begin() + index);
        updateGraphRouting();
    }
}

void AudioEngine::applyAutoPreamp()
{
    m_deviceFilterProcessor.setPreamp(-(m_deviceFilterProcessor.calculateMaxGain() + 0.1f));
}

juce::PluginDescription AudioEngine::getHostedPluginDescription(int index) const
{
    if (auto* instance = dynamic_cast<juce::AudioPluginInstance*>(m_pluginNodes[index]->getProcessor()))
        return instance->getPluginDescription();
    return {};
}

juce::AudioProcessor* AudioEngine::getHostedPluginProcessor(int index)
{
    return m_pluginNodes[index]->getProcessor();
}

} // namespace equinox
