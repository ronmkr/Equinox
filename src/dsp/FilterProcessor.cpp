#include "FilterProcessor.h"
#include <regex>

namespace equinox
{

FilterProcessor::FilterProcessor()
{
    m_settingsA.resize(MaxFilters);
    m_settingsB.resize(MaxFilters);
}

void FilterProcessor::prepareToPlay(double sampleRate, int samplesPerBlock, int numChannels)
{
    m_sampleRate = sampleRate;
    m_numChannels = numChannels;

    juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32)samplesPerBlock, (juce::uint32)numChannels };

    m_filterChainA.prepare(spec);
    m_filterChainB.prepare(spec);
    updateFilterChain();

    juce::FloatVectorOperations::disableDenormalisedNumberSupport();
}

void FilterProcessor::processBlock(juce::AudioBuffer<float>& buffer)
{
    if (m_isBypassed.load())
        return;

    // Apply Preamp
    buffer.applyGain(m_preampGain.load());

    // Process through the chain
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);

    const juce::ScopedLock sl(m_coefficientsLock);
    if (m_isUsingProfileB.load())
        m_filterChainB.process(context);
    else
        m_filterChainA.process(context);
}

void FilterProcessor::reset()
{
    m_filterChainA.reset();
    m_filterChainB.reset();
}

void FilterProcessor::setBypassed(bool shouldBypass)
{
    m_isBypassed.store(shouldBypass);
}

void FilterProcessor::setPreamp(float gainDb)
{
    m_preampGain.store(juce::Decibels::decibelsToGain(gainDb));
}

void FilterProcessor::updateBandGain(int index, float frequency, float gainDb, float q)
{
    const juce::ScopedLock sl(m_coefficientsLock);
    auto& targetSettings = m_isUsingProfileB.load() ? m_settingsB : m_settingsA;
    if (index >= 0 && index < MaxFilters)
    {
        targetSettings[index].enabled = std::abs(gainDb) > 0.01f;
        targetSettings[index].frequency = frequency;
        targetSettings[index].gain = gainDb;
        targetSettings[index].q = q;
        updateFilterChain();
    }
}

void FilterProcessor::toggleAB()
{
    m_isUsingProfileB.store(!m_isUsingProfileB.load());
}

void FilterProcessor::snapshotToOther()
{
    const juce::ScopedLock sl(m_coefficientsLock);
    if (m_isUsingProfileB.load())
        m_settingsA = m_settingsB;
    else
        m_settingsB = m_settingsA;
    updateFilterChain();
}

float FilterProcessor::getMagnitudeForFrequency(float frequency, double sampleRate) const
{
    const juce::ScopedLock sl(m_coefficientsLock);
    auto magnitude = 1.0f;
    const auto& targetSettings = m_isUsingProfileB.load() ? m_settingsB : m_settingsA;
    for (const auto& s : targetSettings)
    {
        if (s.enabled && s.coefficients)
        {
            magnitude *= s.coefficients->getMagnitudeForFrequency(frequency, sampleRate);
        }
    }
    return magnitude;
}

float FilterProcessor::calculateMaxGain() const
{
    float maxMag = 1.0f;
    const int numSteps = 100;
    
    // Check 100 points logarithmic-ish from 20Hz to 20kHz
    for (int i = 0; i <= numSteps; ++i)
    {
        float freq = 20.0f * std::pow(1000.0f, (float)i / numSteps);
        maxMag = std::max(maxMag, getMagnitudeForFrequency(freq, m_sampleRate));
    }
    
    return juce::Decibels::gainToDecibels(maxMag);
}

void FilterProcessor::parseAutoEqString(const std::string& autoEqContent)
{
    const juce::ScopedLock sl(m_coefficientsLock);
    auto& targetSettings = m_isUsingProfileB.load() ? m_settingsB : m_settingsA;

    std::regex preampRegex(R"(Preamp:\s*([-+]?\d*\.?\d+)\s*dB)");
    std::regex filterRegex(R"(Filter\s*(\d+):\s*ON\s*PK\s*Fc\s*(\d*\.?\d+)\s*Hz\s*Gain\s*([-+]?\d*\.?\d+)\s*dB\s*Q\s*(\d*\.?\d+))");

    std::smatch match;

    if (std::regex_search(autoEqContent, match, preampRegex)) {
        setPreamp(std::stof(match[1]));
    }

    auto it = autoEqContent.cbegin();
    while (std::regex_search(it, autoEqContent.cend(), match, filterRegex)) {
        auto index = std::stoi(match[1]) - 1;
        if (index >= 0 && index < MaxFilters) {
            targetSettings[index].enabled = true;
            targetSettings[index].frequency = std::stof(match[2]);
            targetSettings[index].gain = std::stof(match[3]);
            targetSettings[index].q = std::stof(match[4]);
        }
        it = match.suffix().first;
    }

    updateFilterChain();
}

template <size_t Index>
struct FilterChainUpdater {
    static void update(FilterProcessor::FilterChain& chain, double sampleRate, const std::vector<juce::dsp::IIR::Coefficients<float>::Ptr>& coeffs) {
        auto& filter = chain.template get<Index>();
        if (Index < coeffs.size() && coeffs[Index]) {
            filter.coefficients = coeffs[Index];
            chain.setBypassed<Index>(false);
        } else {
            chain.setBypassed<Index>(true);
        }
        if constexpr (Index > 0) {
            FilterChainUpdater<Index - 1>::update(chain, sampleRate, coeffs);
        }
    }
};

void FilterProcessor::updateFilterChain()
{
    auto updateChain = [this](std::vector<FilterSettings>& settings, FilterChain& chain) {
        std::vector<juce::dsp::IIR::Coefficients<float>::Ptr> allCoeffs;
        for (auto& s : settings) {
            if (s.enabled) {
                s.coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(m_sampleRate, s.frequency, s.q, juce::Decibels::decibelsToGain(s.gain));
                allCoeffs.push_back(s.coefficients);
            } else {
                s.coefficients = nullptr;
                allCoeffs.push_back(nullptr);
            }
        }
        FilterChainUpdater<MaxFilters - 1>::update(chain, m_sampleRate, allCoeffs);
    };

    updateChain(m_settingsA, m_filterChainA);
    updateChain(m_settingsB, m_filterChainB);
}

} // namespace equinox
