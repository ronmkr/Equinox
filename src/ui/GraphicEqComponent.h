#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <vector>
#include "../dsp/FilterProcessor.h"

namespace equinox
{

class GraphicEqComponent : public juce::Component
{
public:
    GraphicEqComponent(FilterProcessor& processor);
    ~GraphicEqComponent() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

    std::vector<float> getGains() const;

private:
    FilterProcessor& filterProcessor;
    juce::OwnedArray<juce::Slider> sliders;
    juce::OwnedArray<juce::Label> labels;

    static const std::vector<float> isoFrequencies;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GraphicEqComponent)
};

} // namespace equinox
