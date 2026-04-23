#pragma once

#include <juce_dsp/juce_dsp.h>
#include <juce_audio_processors/juce_audio_processors.h>

namespace equinox
{

/**
 * @class ConvolutionProcessor
 * @brief Manages WAV-based impulse response (IR) convolution.
 */
class ConvolutionProcessor : public juce::AudioProcessor
{
public:
    ConvolutionProcessor()
        : AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true)
                                           .withOutput("Output", juce::AudioChannelSet::stereo(), true))
    {
        m_isEnabled.store(false);
    }

    const juce::String getName() const override { return "Convolution Reverb/IR"; }

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override
    {
        return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo()
            && layouts.getMainInputChannelSet()  == juce::AudioChannelSet::stereo();
    }

    void prepareToPlay(double sampleRate, int samplesPerBlock) override
    {
        juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32)samplesPerBlock, (juce::uint32)getTotalNumOutputChannels() };
        m_convolver.prepare(spec);
    }

    void releaseResources() override {}

    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) override
    {
        if (!m_isEnabled.load() || buffer.getNumChannels() == 0)
            return;

        juce::dsp::AudioBlock<float> block(buffer);
        juce::dsp::ProcessContextReplacing<float> context(block);
        m_convolver.process(context);
    }

    void loadImpulseResponse(const juce::File& file)
    {
        if (file.existsAsFile())
        {
            m_convolver.loadImpulseResponse(file,
                                            juce::dsp::Convolution::Stereo::yes,
                                            juce::dsp::Convolution::Trim::yes,
                                            0,
                                            juce::dsp::Convolution::Normalise::yes);
            m_isEnabled.store(true);
        }
    }

    void setEnabled(bool enabled) { m_isEnabled.store(enabled); }
    [[nodiscard]] bool isEnabled() const { return m_isEnabled.load(); }

    // Mandatory juce::AudioProcessor overrides
    double getTailLengthSeconds() const override { return 0.0; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    juce::AudioProcessorEditor* createEditor() override { return nullptr; }
    bool hasEditor() const override { return false; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override {}
    void setStateInformation(const void*, int) override {}

private:
    juce::dsp::Convolution m_convolver { juce::dsp::Convolution::Latency { 0 } };
    std::atomic<bool> m_isEnabled;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ConvolutionProcessor)
};

} // namespace equinox
