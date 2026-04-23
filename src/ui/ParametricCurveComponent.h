#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../dsp/FilterProcessor.h"
#include "../dsp/AudioEngine.h"

namespace equinox
{

class ParametricCurveComponent : public juce::Component, public juce::Timer
{
public:
    ParametricCurveComponent(FilterProcessor& processor, AudioEngine& engine);
    ~ParametricCurveComponent() override = default;

    void paint(juce::Graphics& g) override;
    void timerCallback() override;
    
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;

private:
    FilterProcessor& filterProcessor;
    AudioEngine& m_audioEngine;
    juce::Path responsePath;
    
    int m_draggingIndex = -1;
    
    juce::Point<float> getPosForFreq(float freq, float gain);
    float getFreqForPos(float x);
    float getGainForPos(float y);

    void updatePath();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ParametricCurveComponent)
};

} // namespace equinox
