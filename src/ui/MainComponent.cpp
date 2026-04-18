#include "MainComponent.h"
#include "../dsp/CrossfeedProcessor.h"

namespace equinox
{

// Define layout structs inside namespace for easier access
struct MainComponent::EqLayout : public juce::Component {
    GraphicEqComponent& geq;
    ParametricCurveComponent& curve;
    EqLayout(GraphicEqComponent& g, ParametricCurveComponent& c) : geq(g), curve(c) {
        addAndMakeVisible(geq);
        addAndMakeVisible(curve);
    }
    void resized() override {
        auto r = getLocalBounds();
        curve.setBounds(r.removeFromTop(200));
        geq.setBounds(r);
    }
};

struct MainComponent::PluginsLayout : public juce::Component {
    MainComponent& owner;
    PluginsLayout(MainComponent& o) : owner(o) {
        addAndMakeVisible(owner.knownPluginsList);
        addAndMakeVisible(owner.activePluginsList);
        addAndMakeVisible(owner.scanButton);
        addAndMakeVisible(owner.addButton);
        addAndMakeVisible(owner.removeButton);
    }
    void resized() override {
        auto r = getLocalBounds().reduced(10);
        auto left = r.removeFromLeft(r.getWidth() / 2).reduced(5);
        auto right = r.reduced(5);
        
        auto leftButtons = left.removeFromBottom(40);
        owner.scanButton.setBounds(leftButtons.removeFromLeft(120).reduced(2));
        owner.addButton.setBounds(leftButtons.reduced(2));
        owner.knownPluginsList.setBounds(left);

        auto rightButtons = right.removeFromBottom(40);
        owner.removeButton.setBounds(rightButtons.reduced(2));
        owner.activePluginsList.setBounds(right);
    }
};

struct MainComponent::SettingsLayout : public juce::Component {
    juce::AudioDeviceSelectorComponent& selector;
    juce::ToggleButton& cfToggle;
    juce::Slider& cfAmount;
    juce::ToggleButton& apToggle;

    SettingsLayout(juce::AudioDeviceSelectorComponent& s, juce::ToggleButton& cft, juce::Slider& cfa, juce::ToggleButton& apt)
        : selector(s), cfToggle(cft), cfAmount(cfa), apToggle(apt)
    {
        addAndMakeVisible(selector);
        addAndMakeVisible(cfToggle);
        addAndMakeVisible(cfAmount);
        addAndMakeVisible(apToggle);
    }

    void resized() override {
        auto r = getLocalBounds().reduced(10);
        selector.setBounds(r.removeFromTop(300));
        
        auto extra = r.reduced(10);
        cfToggle.setBounds(extra.removeFromTop(30));
        cfAmount.setBounds(extra.removeFromTop(30));
        apToggle.setBounds(extra.removeFromTop(30));
    }
};

MainComponent::MainComponent(AudioEngine& engine)
    : audioEngine(engine),
      tabs(juce::TabbedButtonBar::TabsAtTop),
      graphicEq(engine.getFilterProcessor()),
      curveComponent(engine.getFilterProcessor()),
      deviceSelector(engine.getDeviceManager(), 2, 2, 2, 2, true, true, true, false),
      knownPluginsModel(engine),
      activePluginsModel(engine)
{
    // Profile Controls
    addAndMakeVisible(saveButton);
    saveButton.onClick = [this] {
        EqProfile p;
        p.name = "New Profile " + juce::String(profileList.getNumItems() + 1);
        p.gains = graphicEq.getGains();
        profileManager.saveProfile(p);
        profileList.addItem(p.name, profileList.getNumItems() + 1);
    };

    addAndMakeVisible(abButton);
    abButton.onClick = [this] {
        audioEngine.getFilterProcessor().toggleAB();
    };

    addAndMakeVisible(profileList);
    profileList.setTextWhenNoChoicesAvailable("No Profiles");

    // Layout Initializations
    eqLayout = std::make_unique<EqLayout>(graphicEq, curveComponent);
    pluginsLayout = std::make_unique<PluginsLayout>(*this);

    // Crossfeed / Preamp Config
    crossfeedToggle.setToggleState(false, juce::NotificationType::dontSendNotification);
    crossfeedToggle.onClick = [this] {
        if (auto* cf = dynamic_cast<equinox::CrossfeedProcessor*>(audioEngine.getCrossfeedNode()->getProcessor()))
            cf->setEnabled(crossfeedToggle.getToggleState());
    };

    crossfeedAmount.setRange(0.0, 1.0, 0.01);
    crossfeedAmount.setValue(0.3);
    crossfeedAmount.onValueChange = [this] {
        if (auto* cf = dynamic_cast<equinox::CrossfeedProcessor*>(audioEngine.getCrossfeedNode()->getProcessor()))
            cf->setAmount((float)crossfeedAmount.getValue());
    };

    autoPreampToggle.setToggleState(false, juce::NotificationType::dontSendNotification);
    autoPreampToggle.onClick = [this] {
        if (autoPreampToggle.getToggleState())
            audioEngine.applyAutoPreamp();
        else
            audioEngine.getFilterProcessor().setPreamp(0.0f);
    };

    settingsLayout = std::make_unique<SettingsLayout>(deviceSelector, crossfeedToggle, crossfeedAmount, autoPreampToggle);

    // Plugins Tab Config
    knownPluginsList.setModel(&knownPluginsModel);
    activePluginsList.setModel(&activePluginsModel);

    pluginsContainer.addAndMakeVisible(knownPluginsList);
    pluginsContainer.addAndMakeVisible(activePluginsList);
    pluginsContainer.addAndMakeVisible(scanButton);
    pluginsContainer.addAndMakeVisible(addButton);
    pluginsContainer.addAndMakeVisible(removeButton);

    scanButton.onClick = [this] {
        audioEngine.scanPlugins();
        knownPluginsList.updateContent();
    };

    addButton.onClick = [this] {
        auto row = knownPluginsList.getSelectedRow();
        if (row >= 0) {
            auto& list = audioEngine.getKnownPluginList();
            if (audioEngine.addPlugin(*list.getType(row))) {
                activePluginsList.updateContent();
            }
        }
    };

    removeButton.onClick = [this] {
        auto row = activePluginsList.getSelectedRow();
        if (row >= 0) {
            audioEngine.removePlugin(row);
            activePluginsList.updateContent();
        }
    };

    tabs.addTab("Equalizer", juce::Colours::darkgrey, eqLayout.get(), false);
    tabs.addTab("Plugins", juce::Colours::darkgrey, pluginsLayout.get(), false);
    tabs.addTab("Settings", juce::Colours::darkgrey, settingsLayout.get(), false);

    addAndMakeVisible(tabs);
    setSize(800, 600);
}

void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void MainComponent::resized()
{
    auto area = getLocalBounds();
    auto topBar = area.removeFromTop(40).reduced(5);
    
    profileList.setBounds(topBar.removeFromLeft(150));
    saveButton.setBounds(topBar.removeFromLeft(80).reduced(2));
    abButton.setBounds(topBar.removeFromLeft(100).reduced(2));

    tabs.setBounds(area);
}

void MainComponent::KnownPluginsModel::paintListBoxItem(int row, juce::Graphics& g, int width, int height, bool selected)
{
    if (selected) g.fillAll(juce::Colours::lightblue);
    g.setColour(selected ? juce::Colours::black : juce::Colours::white);
    auto* type = engine.getKnownPluginList().getType(row);
    if (type) g.drawText(type->name + " (" + type->pluginFormatName + ")", 5, 0, width, height, juce::Justification::centredLeft);
}

void MainComponent::ActivePluginsModel::paintListBoxItem(int row, juce::Graphics& g, int width, int height, bool selected)
{
    if (selected) g.fillAll(juce::Colours::lightblue);
    g.setColour(selected ? juce::Colours::black : juce::Colours::white);
    if (row < engine.getHostedPluginCount())
    {
        auto desc = engine.getHostedPluginDescription(row);
        g.drawText(desc.name, 5, 0, width, height, juce::Justification::centredLeft);
    }
}

} // namespace equinox
