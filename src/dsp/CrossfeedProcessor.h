#pragma once

#include <juce_dsp/juce_dsp.h>
#include <juce_audio_processors/juce_audio_processors.h>

namespace equinox
{

/**
 * @class CrossfeedProcessor
 * @brief Implements a simple headphone crossfeed algorithm to reduce stereo fatigue.
 * 
 * This uses a basic Linkwitz-style crossfeed:
 * 1. Low-pass filter the opposite channel.
 * 2. Delay it slightly (~250-400 microseconds).
 * 3. Mix it with the current channel.
 */
class CrossfeedProcessor : public juce::AudioProcessor
{
public:
    CrossfeedProcessor()
    {
        m_crossfeedAmount.store(0.3f); // Default 30% crossfeed
        m_isEnabled.store(false);      // Default to off
    }

    const juce::String getName() const override { return "Headphone Crossfeed"; }

    void prepareToPlay(double sampleRate, int samplesPerBlock) override
    {
        m_sampleRate = sampleRate;
        
        juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32)samplesPerBlock, 2 };
        
        // Setup Low-Pass filters for the crossfeed signal
        // We want a gentle roll-off around 700-800Hz
        m_lowPassL.prepare(spec);
        m_lowPassR.prepare(spec);
        
        auto coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, 700.0f, 0.5f);
        m_lowPassL.coefficients = coefficients;
        m_lowPassR.coefficients = coefficients;

        // Setup Delays (~300 microseconds)
        m_delayLineL.prepare(spec);
        m_delayLineR.prepare(spec);
        
        auto delayInSamples = (float)(0.0003 * sampleRate);
        m_delayLineL.setDelay(delayInSamples);
        m_delayLineR.setDelay(delayInSamples);
    }

    void releaseResources() override {}

    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) override
    {
        if (!m_isEnabled.load() || buffer.getNumChannels() < 2)
            return;

        auto amount = m_crossfeedAmount.load();
        auto numSamples = buffer.getNumSamples();
        
        auto* left = buffer.getWritePointer(0);
        auto* right = buffer.getWritePointer(1);

        for (int i = 0; i < numSamples; ++i)
        {
            // 1. Get current samples
            float lIn = left[i];
            float rIn = right[i];

            // 2. Process crossfeed path (opposite channel -> LPF -> Delay)
            m_delayLineL.pushSample(0, rIn);
            m_delayLineR.pushSample(0, lIn);
            
            float rCross = m_delayLineL.popSample(0);
            float lCross = m_delayLineR.popSample(0);
            
            rCross = m_lowPassL.processSample(rCross);
            lCross = m_lowPassR.processSample(lCross);

            // 3. Mix into the current channel
            left[i] = lIn + (rCross * amount);
            right[i] = rIn + (lCross * amount);
            
            // Apply a small attenuation to prevent clipping from the sum
            left[i] *= 0.85f;
            right[i] *= 0.85f;
        }
    }

    void setEnabled(bool enabled) { m_isEnabled.store(enabled); }
    void setAmount(float amount) { m_crossfeedAmount.store(juce::jlimit(0.0f, 1.0f, amount)); }

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
    std::atomic<float> m_crossfeedAmount;
    std::atomic<bool> m_isEnabled;
    double m_sampleRate = 44100.0;

    juce::dsp::IIR::Filter<float> m_lowPassL;
    juce::dsp::IIR::Filter<float> m_lowPassR;
    
    juce::dsp::DelayLine<float> m_delayLineL { 1024 };
    juce::dsp::DelayLine<float> m_delayLineR { 1024 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CrossfeedProcessor)
};

} // namespace equinox
