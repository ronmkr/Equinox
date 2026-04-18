#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "../dsp/AudioEngine.h"
#include "../dsp/ProfileManager.h"
#include "GraphicEqComponent.h"
#include "ParametricCurveComponent.h"

namespace equinox
{

class MainComponent : public juce::Component
{
public:
    MainComponent(AudioEngine& engine);
    ~MainComponent() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    AudioEngine& audioEngine;
    ProfileManager profileManager;
    
    juce::TabbedComponent tabs;
    
    // EQ Tab controls
    juce::TextButton saveButton { "Save" };
    juce::TextButton abButton { "A/B Toggle" };
    juce::ComboBox profileList;
    
    GraphicEqComponent graphicEq;
    ParametricCurveComponent curveComponent;
    
    juce::AudioDeviceSelectorComponent deviceSelector;

    // Additional Settings
    juce::ToggleButton crossfeedToggle { "Enable Headphone Crossfeed" };
    juce::Slider crossfeedAmount { juce::Slider::LinearHorizontal, juce::Slider::NoTextBox };
    juce::ToggleButton autoPreampToggle { "Auto-Preamp (Avoid Clipping)" };

    // Plugins Tab
    juce::Component pluginsContainer;
    juce::ListBox knownPluginsList;
    juce::ListBox activePluginsList;
    juce::TextButton scanButton { "Scan for Plugins" };
    juce::TextButton addButton { "Add Selected" };
    juce::TextButton removeButton { "Remove Selected" };

    struct EqLayout;
    struct PluginsLayout;
    struct SettingsLayout;
    std::unique_ptr<EqLayout> eqLayout;
    std::unique_ptr<PluginsLayout> pluginsLayout;
    std::unique_ptr<SettingsLayout> settingsLayout;

    struct KnownPluginsModel : public juce::ListBoxModel {
        AudioEngine& engine;
        KnownPluginsModel(AudioEngine& e) : engine(e) {}
        int getNumRows() override { return engine.getKnownPluginList().getNumTypes(); }
        void paintListBoxItem(int row, juce::Graphics& g, int width, int height, bool selected) override;
    } knownPluginsModel;

    struct ActivePluginsModel : public juce::ListBoxModel {
        AudioEngine& engine;
        ActivePluginsModel(AudioEngine& e) : engine(e) {}
        int getNumRows() override { /* will need a way to get count from engine */ return 0; }
        void paintListBoxItem(int row, juce::Graphics& g, int width, int height, bool selected) override;
    } activePluginsModel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};

} // namespace equinox
