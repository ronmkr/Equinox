#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../dsp/FilterProcessor.h"

namespace equinox
{

class ParametricCurveComponent : public juce::Component, public juce::Timer
{
public:
    ParametricCurveComponent(FilterProcessor& processor);
    ~ParametricCurveComponent() override = default;

    void paint(juce::Graphics& g) override;
    void timerCallback() override;

private:
    FilterProcessor& filterProcessor;
    juce::Path responsePath;

    void updatePath();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ParametricCurveComponent)
};

} // namespace equinox
