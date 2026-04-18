#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "../dsp/AudioEngine.h"
#include "../dsp/ProfileManager.h"
#include "GraphicEqComponent.h"
#include "ParametricCurveComponent.h"
#include "TahoeLookAndFeel.h"

namespace equinox
{

class MainComponent : public juce::Component, public juce::Timer
{
public:
    MainComponent(AudioEngine& engine);
    ~MainComponent() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;
    
    void timerCallback() override
    {
        tahoeLookAndFeel.refreshColours();
        repaint();
    }

private:
    AudioEngine& audioEngine;
    ProfileManager profileManager;
    
    TahoeLookAndFeel tahoeLookAndFeel;
    juce::TabbedComponent tabs;
    
    // EQ Tab controls
    juce::TextButton saveButton { "Save" };
    juce::TextButton abButton { "A/B Toggle" };
    juce::TextButton resetEqButton { "Reset EQ" };
    juce::ToggleButton globalBypassToggle { "Global Bypass" };
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
    juce::TextButton editButton { "Edit Plugin" };

    struct EqLayout;
    struct PluginsLayout;
    struct SettingsLayout;
    std::unique_ptr<EqLayout> eqLayout;
    std::unique_ptr<PluginsLayout> pluginsLayout;
    std::unique_ptr<SettingsLayout> settingsLayout;

    // Plugin Editor management
    class PluginWindow : public juce::DocumentWindow
    {
    public:
        PluginWindow(juce::AudioProcessor& p)
            : DocumentWindow(p.getName(), juce::Colours::black, DocumentWindow::allButtons)
        {
            setUsingNativeTitleBar(true);
            if (auto* editor = p.createEditorIfNeeded())
            {
                setContentOwned(editor, true);
                setResizable(editor->isResizable(), false);
            }
            setVisible(true);
        }
        void closeButtonPressed() override { delete this; }
    };
    juce::Component::SafePointer<juce::Component> activePluginWindow;

    struct KnownPluginsModel : public juce::ListBoxModel {
        AudioEngine& engine;
        KnownPluginsModel(AudioEngine& e) : engine(e) {}
        int getNumRows() override;
        void paintListBoxItem(int row, juce::Graphics& g, int width, int height, bool selected) override;
    };
    KnownPluginsModel knownPluginsModel;

    struct ActivePluginsModel : public juce::ListBoxModel {
        AudioEngine& engine;
        ActivePluginsModel(AudioEngine& e) : engine(e) {}
        int getNumRows() override;
        void paintListBoxItem(int row, juce::Graphics& g, int width, int height, bool selected) override;
    };
    ActivePluginsModel activePluginsModel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};

} // namespace equinox
