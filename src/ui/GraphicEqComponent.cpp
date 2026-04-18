#include "GraphicEqComponent.h"

namespace equinox
{

const std::vector<float> GraphicEqComponent::isoFrequencies = {
    20, 25, 31.5, 40, 50, 63, 80, 100, 125, 160, 200, 250, 315, 400, 500, 630, 800, 1000, 1250, 1600, 2000, 2500, 3150, 4000, 5000, 6300, 8000, 10000, 12500, 16000, 20000
};

GraphicEqComponent::GraphicEqComponent(FilterProcessor& processor)
    : filterProcessor(processor)
{
    for (size_t i = 0; i < isoFrequencies.size(); ++i)
    {
        auto* slider = sliders.add(new juce::Slider(juce::Slider::LinearVertical, juce::Slider::NoTextBox));
        slider->setRange(-15.0, 15.0, 0.1);
        slider->setValue(0.0);
        slider->onValueChange = [this, i, slider] {
            filterProcessor.updateBandGain((int)i, isoFrequencies[i], (float)slider->getValue(), 1.41f);
        };
        addAndMakeVisible(slider);

        auto freqStr = (isoFrequencies[i] >= 1000) ? 
            juce::String(isoFrequencies[i] / 1000.0f, 1) + "k" : 
            juce::String((int)isoFrequencies[i]);

        auto* label = labels.add(new juce::Label(freqStr, freqStr));
        label->setJustificationType(juce::Justification::centred);
        label->setFont(juce::FontOptions(10.0f));
        addAndMakeVisible(label);
    }
}

void GraphicEqComponent::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void GraphicEqComponent::resized()
{
    auto area = getLocalBounds().reduced(10);
    auto sliderWidth = area.getWidth() / (int)isoFrequencies.size();
    
    for (int i = 0; i < sliders.size(); ++i)
    {
        auto sliderArea = area.removeFromLeft(sliderWidth);
        labels[i]->setBounds(sliderArea.removeFromBottom(20));
        sliders[i]->setBounds(sliderArea);
    }
}

std::vector<float> GraphicEqComponent::getGains() const
{
    std::vector<float> gains;
    for (auto* s : sliders)
        gains.push_back((float)s->getValue());
    return gains;
}

} // namespace equinox
