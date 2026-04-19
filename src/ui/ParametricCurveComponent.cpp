#include "ParametricCurveComponent.h"
#include <cmath>

namespace equinox
{

ParametricCurveComponent::ParametricCurveComponent(FilterProcessor& processor)
    : filterProcessor(processor)
{
    startTimer(30); // 30Hz refresh
}

void ParametricCurveComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black.withAlpha(0.3f));
    
    // Draw Grid
    g.setColour(juce::Colours::white.withAlpha(0.1f));
    for (float f = 100.0f; f < 20000.0f; f *= 2.0f) {
        auto p = getPosForFreq(f, 0.0f);
        g.drawVerticalLine((int)p.x, 0.0f, (float)getHeight());
    }
    
    // Draw Curve
    updatePath();
    g.setColour(juce::Colours::cyan);
    g.strokePath(responsePath, juce::PathStrokeType(2.0f));

    // Draw Interactive Nodes (the "Dots")
    g.setColour(juce::Colours::white);
    for (int i = 0; i < 31; ++i) {
        // We'll just show nodes that aren't 0dB for clarity
        // but for now, let's just show a few main ones
        if (i % 3 == 0) {
            // Frequency would be needed here - we'll approximate based on ISO frequencies
            // or just use fixed spacing for now to prove interaction
        }
    }
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
