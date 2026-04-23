#pragma once

#include <juce_dsp/juce_dsp.h>
#include <juce_audio_processors/juce_audio_processors.h>

namespace equinox
{

/**
 * @class LoudnessProcessor
 * @brief Implements ISO 226:2003 inspired dynamic loudness compensation.
 * 
 * As volume decreases, the human ear becomes less sensitive to low and high 
 * frequencies. This processor applies a compensatory "smile" curve that 
 * scales automatically with the master volume level.
 */
class LoudnessProcessor : public juce::AudioProcessor
{
public:
    LoudnessProcessor()
        : AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true)
                                           .withOutput("Output", juce::AudioChannelSet::stereo(), true))
    {
        m_isEnabled.store(false);
        m_currentVolumeDb.store(0.0f);
    }

    const juce::String getName() const override { return "Dynamic Loudness"; }

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override
    {
        return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo()
            && layouts.getMainInputChannelSet()  == juce::AudioChannelSet::stereo();
    }

    void prepareToPlay(double sampleRate, int samplesPerBlock) override
    {
        m_sampleRate = sampleRate;
        juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32)samplesPerBlock, (juce::uint32)getTotalNumOutputChannels() };
        
        m_lowShelf.prepare(spec);
        m_highShelf.prepare(spec);
        
        updateCoefficients();
    }

    void releaseResources() override {}

    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) override
    {
        if (!m_isEnabled.load() || buffer.getNumChannels() == 0)
            return;

        juce::dsp::AudioBlock<float> block(buffer);
        juce::dsp::ProcessContextReplacing<float> context(block);
        
        m_lowShelf.process(context);
        m_highShelf.process(context);
    }

    void setEnabled(bool enabled) { m_isEnabled.store(enabled); }
    [[nodiscard]] bool isEnabled() const { return m_isEnabled.load(); }

    /**
     * @brief Updates the compensation based on the current volume in dB.
     * Expects values roughly in the range of -60.0 to 0.0.
     */
    void setVolumeDb(float volumeDb)
    {
        if (std::abs(m_currentVolumeDb.load() - volumeDb) > 0.1f)
        {
            m_currentVolumeDb.store(volumeDb);
            updateCoefficients();
        }
    }

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
    void updateCoefficients()
    {
        if (m_sampleRate <= 0) return;

        // Simplified ISO 226 approximation:
        // As volume drops below 0dB, we boost bass and treble.
        float vol = juce::jlimit(-60.0f, 0.0f, m_currentVolumeDb.load());
        float deficit = std::abs(vol);
        
        float bassBoostDb = deficit * 0.25f;    // Max ~15dB boost at -60dB vol
        float trebleBoostDb = deficit * 0.125f; // Max ~7.5dB boost at -60dB vol

        m_lowShelf.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowShelf(m_sampleRate, 100.0f, 0.707f, juce::Decibels::decibelsToGain(bassBoostDb));
        m_highShelf.coefficients = juce::dsp::IIR::Coefficients<float>::makeHighShelf(m_sampleRate, 10000.0f, 0.707f, juce::Decibels::decibelsToGain(trebleBoostDb));
    }

    double m_sampleRate = 44100.0;
    std::atomic<bool> m_isEnabled;
    std::atomic<float> m_currentVolumeDb;

    juce::dsp::IIR::Filter<float> m_lowShelf;
    juce::dsp::IIR::Filter<float> m_highShelf;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LoudnessProcessor)
};

} // namespace equinox
