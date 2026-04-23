#include "ParametricCurveComponent.h"
#include <cmath>

namespace equinox
{

ParametricCurveComponent::ParametricCurveComponent(FilterProcessor& processor, AudioEngine& engine)
    : filterProcessor(processor), m_audioEngine(engine)
{
    startTimer(30); // 30Hz refresh
}

void ParametricCurveComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black.withAlpha(0.3f));
    
    // --- Draw FFT Spectrum ---
    auto* fftData = m_audioEngine.getFFTData();
    auto fftSize = AudioEngine::fftSize;
    
    juce::Path fftPath;
    bool firstPoint = true;
    
    for (int i = 2; i < fftSize / 2; ++i)
    {
        float freq = (float)i * (44100.0f / (float)fftSize);
        auto p = getPosForFreq(freq, 0.0f); // Base Y is middle
        
        // Log-scaled magnitude conversion for visualization
        float mag = std::abs(fftData[i]);
        float level = 0.0f;
        if (mag > 0.00001f)
            level = juce::jlimit(0.0f, 1.0f, (juce::Decibels::gainToDecibels(mag) + 100.0f) / 100.0f);

        float yPos = (float)getHeight() - (level * (float)getHeight());
        
        if (firstPoint) {
            fftPath.startNewSubPath(p.x, yPos);
            firstPoint = false;
        } else {
            fftPath.lineTo(p.x, yPos);
        }
    }
    
    g.setColour(juce::Colours::white.withAlpha(0.15f));
    g.strokePath(fftPath, juce::PathStrokeType(1.0f));
    
    // --- Draw Grid ---
    g.setColour(juce::Colours::white.withAlpha(0.05f));
    for (float f = 100.0f; f < 20000.0f; f *= 2.0f) {
        auto p = getPosForFreq(f, 0.0f);
        g.drawVerticalLine((int)p.x, 0.0f, (float)getHeight());
    }
    
    // --- Draw Curve ---
    updatePath();
    g.setColour(juce::Colours::cyan.withAlpha(0.8f));
    g.strokePath(responsePath, juce::PathStrokeType(2.0f));
}

void ParametricCurveComponent::timerCallback()
{
    repaint();
}

juce::Point<float> ParametricCurveComponent::getPosForFreq(float freq, float gain)
{
    auto x = std::log10(freq / 20.0f) / std::log10(20000.0f / 20.0f) * getWidth();
    auto y = getHeight() * 0.5f - (gain * (getHeight() * 0.4f) / 15.0f);
    return { (float)x, (float)y };
}

float ParametricCurveComponent::getFreqForPos(float x)
{
    return 20.0f * std::pow(1000.0f, x / getWidth());
}

float ParametricCurveComponent::getGainForPos(float y)
{
    return (getHeight() * 0.5f - y) * 15.0f / (getHeight() * 0.4f);
}

void ParametricCurveComponent::mouseDown(const juce::MouseEvent& e)
{
    // Find closest band to mouse click
    // Simplified: Check which of the 31 bands is closest to e.x
    float minSourceDist = 100.0f;
    m_draggingIndex = -1;

    for (int i = 0; i < 31; ++i)
    {
        // Frequency approximate logic
        float freq = 20.0f * std::pow(1000.0f, (float)i / 30.0f);
        auto p = getPosForFreq(freq, 0.0f); // Check X only for now
        float dist = std::abs(p.x - e.x);
        if (dist < minSourceDist && dist < 20.0f) {
            minSourceDist = dist;
            m_draggingIndex = i;
        }
    }
}

void ParametricCurveComponent::mouseDrag(const juce::MouseEvent& e)
{
    if (m_draggingIndex >= 0)
    {
        float freq = 20.0f * std::pow(1000.0f, (float)m_draggingIndex / 30.0f);
        float gain = juce::jlimit(-15.0f, 15.0f, getGainForPos((float)e.y));
        
        filterProcessor.updateBandGain(m_draggingIndex, freq, gain, 1.41f);
    }
}

void ParametricCurveComponent::updatePath()
{
    responsePath.clear();
    auto w = (float)getWidth();
    auto h = (float)getHeight();

    for (float x = 0; x < w; x += 2.0f)
    {
        float freq = getFreqForPos(x);
        float mag = filterProcessor.getMagnitudeForFrequency(freq, 44100.0);
        float gain = juce::Decibels::gainToDecibels(mag);
        auto p = getPosForFreq(freq, gain);

        if (x == 0) responsePath.startNewSubPath(p);
        else responsePath.lineTo(p);
    }
}

} // namespace equinox
