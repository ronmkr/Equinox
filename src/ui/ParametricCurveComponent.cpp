#include "ParametricCurveComponent.h"

namespace equinox
{

ParametricCurveComponent::ParametricCurveComponent(FilterProcessor& processor)
    : filterProcessor(processor)
{
    startTimerHz(30); // 30 FPS update for the curve
}

void ParametricCurveComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);

    // Draw grid lines
    g.setColour(juce::Colours::grey.withAlpha(0.3f));
    for (float f = 20.0f; f <= 20000.0f; f *= 2.0f) {
        auto x = juce::mapFromLog10(f, 20.0f, 20000.0f) * getWidth();
        g.drawVerticalLine((int)x, 0.0f, (float)getHeight());
    }

    for (float db = -15.0f; db <= 15.0f; db += 5.0f) {
        auto y = juce::jmap(db, -15.0f, 15.0f, (float)getHeight(), 0.0f);
        g.drawHorizontalLine((int)y, 0.0f, (float)getWidth());
    }

    // Draw Response Path
    updatePath();
    g.setColour(juce::Colours::cyan);
    g.strokePath(responsePath, juce::PathStrokeType(2.0f));
}

void ParametricCurveComponent::timerCallback()
{
    repaint();
}

void ParametricCurveComponent::updatePath()
{
    responsePath.clear();
    const int numPoints = getWidth();
    const float sampleRate = 48000.0f; // Simplified for now

    for (int i = 0; i < numPoints; ++i)
    {
        float freq = juce::mapToLog10((float)i / numPoints, 20.0f, 20000.0f);
        float mag = filterProcessor.getMagnitudeForFrequency(freq, sampleRate);
        float db = juce::Decibels::gainToDecibels(mag);
        
        float y = juce::jmap(db, -15.0f, 15.0f, (float)getHeight(), 0.0f);

        if (i == 0)
            responsePath.startNewSubPath(0, y);
        else
            responsePath.lineTo((float)i, y);
    }
}

} // namespace equinox
