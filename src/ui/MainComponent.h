#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "../dsp/AudioEngine.h"
#include "../dsp/ProfileManager.h"
#include "GraphicEqComponent.h"
#include "ParametricCurveComponent.h"
#include "OscilloscopeComponent.h"
#include "TahoeLookAndFeel.h"

namespace equinox
{

class MainComponent : public juce::Component, public juce::Timer
{
public:
    MainComponent(AudioEngine& engine);
    ~MainComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;

    // --- Header Section ---
    struct HeaderSection : public juce::Component {
        juce::Label title { "EQUINOX PRO", "EQUINOX PRO" };
        juce::TextButton settingsButton { "SETTINGS" };
        juce::ProgressBar& inMeter;
        juce::ProgressBar& outMeter;

        HeaderSection(juce::ProgressBar& i, juce::ProgressBar& o) : inMeter(i), outMeter(o) {
            addAndMakeVisible(title);
            addAndMakeVisible(settingsButton);
            addAndMakeVisible(inMeter);
            addAndMakeVisible(outMeter);
        }
        void resized() override {
            auto r = getLocalBounds().reduced(10);
            title.setBounds(r.removeFromLeft(150));
            settingsButton.setBounds(r.removeFromLeft(80).reduced(0, 5));
            outMeter.setBounds(r.removeFromRight(120).reduced(0, 5));
            inMeter.setBounds(r.removeFromRight(120).reduced(0, 5));
        }
    };

    // --- Public UI Members ---
    // (Ordered correctly for initialization)
    double m_inputLevel = 0.0;
    double m_outputLevel = 0.0;
    juce::ProgressBar inputMeter { m_inputLevel };
    juce::ProgressBar outputMeter { m_outputLevel };
    HeaderSection header;

    juce::Slider masterFader { juce::Slider::LinearVertical, juce::Slider::TextBoxBelow };
    juce::Label masterLabel { "master", "MASTER" };

    ParametricCurveComponent curveComponent;
    GraphicEqComponent graphicEq;
    OscilloscopeComponent oscilloscope;
    
    juce::GroupComponent inputStrip { "input", "INPUT STAGE" };
    juce::GroupComponent eqStrip { "eq", "PARAMETRIC EQ" };
    juce::GroupComponent masterStrip { "master_strip", "MASTER" };
    
    juce::ToggleButton crossfeedToggle { "CROSSFEED" };
    juce::ToggleButton compressorToggle { "DYNAMICS" };
    juce::ToggleButton convolutionToggle { "CONVOLUTION" };
    juce::ToggleButton bypassToggle { "BYPASS" };
    
    juce::TextButton hd600Button { "AUTOEQ: HD 600" };
    juce::TextButton resetBtn { "FLATTEN" };

    juce::AudioDeviceSelectorComponent deviceSelector;
    bool m_isSettingsOpen = false;

private:
    AudioEngine& m_audioEngine;
    ProfileManager m_profileManager;
    TahoeLookAndFeel m_tahoeLookAndFeel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};

} // namespace equinox
