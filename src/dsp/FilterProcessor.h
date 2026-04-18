#pragma once

#include <juce_dsp/juce_dsp.h>
#include <vector>
#include <string>

namespace equinox
{

/**
 * @class FilterProcessor
 * @brief Manages a cascade of up to 31 Biquad filters for equalization.
 */
class FilterProcessor
{
public:
    FilterProcessor();
    ~FilterProcessor() = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock, int numChannels);
    void processBlock(juce::AudioBuffer<float>& buffer);
    void reset();

    /**
     * @brief Parses an AutoEQ string and updates filter coefficients.
     * Use std::string_view for efficient read-only access where possible, 
     * though regex requires string/iterators.
     */
    void parseAutoEqString(const std::string& autoEqContent);

    void setPreamp(float gainDb);
    void updateBandGain(int index, float frequency, float gainDb, float q);
    void toggleAB();
    void snapshotToOther();

    [[nodiscard]] float getMagnitudeForFrequency(float frequency, double sampleRate) const;

    using Filter = juce::dsp::IIR::Filter<float>;
    using FilterChain = juce::dsp::ProcessorChain<
        Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter,
        Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter,
        Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter, Filter,
        Filter>;

private:
    struct FilterSettings
    {
        bool enabled = false;
        float frequency = 1000.0f;
        float gain = 0.0f;
        float q = 1.0f;
        juce::dsp::IIR::Coefficients<float>::Ptr coefficients;
    };

    void updateFilterChain();

    static constexpr int MaxFilters = 31;
    double m_sampleRate = 44100.0;
    int m_numChannels = 2;
    std::atomic<float> m_preampGain { 1.0f };
    std::atomic<bool> m_isUsingProfileB { false };

    FilterChain m_filterChainA;
    FilterChain m_filterChainB;
    std::vector<FilterSettings> m_settingsA;
    std::vector<FilterSettings> m_settingsB;

    // Mutex for UI-to-DSP parameter updates
    juce::CriticalSection m_coefficientsLock;
};

} // namespace equinox
