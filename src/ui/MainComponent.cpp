#include "MainComponent.h"
#include "../dsp/CrossfeedProcessor.h"

namespace equinox
{

struct MainComponent::EqLayout : public juce::Component {
    GraphicEqComponent& geq;
    ParametricCurveComponent& curve;
    EqLayout(GraphicEqComponent& g, ParametricCurveComponent& c) : geq(g), curve(c) {
        addAndMakeVisible(geq);
        addAndMakeVisible(curve);
    }
    void paint(juce::Graphics& g) override {
        auto r = getLocalBounds().removeFromTop(200).reduced(10);
        g.setColour(juce::Colour(0xff111111));
        g.fillRoundedRectangle(r.toFloat(), 10.0f);
    }
    void resized() override {
        auto r = getLocalBounds();
        auto visualizerArea = r.removeFromTop(200).reduced(15);
        curve.setBounds(visualizerArea);
        geq.setBounds(r.reduced(10, 0));
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
        addAndMakeVisible(owner.editButton);
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
        owner.editButton.setBounds(rightButtons.removeFromLeft(100).reduced(2));
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

int MainComponent::KnownPluginsModel::getNumRows() { return engine.getKnownPluginList().getNumTypes(); }
void MainComponent::KnownPluginsModel::paintListBoxItem(int row, juce::Graphics& g, int width, int height, bool selected)
{
    if (selected) g.fillAll(juce::Colours::lightblue);
    g.setColour(selected ? juce::Colours::black : juce::Colours::white);
    auto& list = engine.getKnownPluginList();
    if (row < list.getNumTypes())
    {
        auto type = list.getTypes()[row];
        g.drawText(type.name + " (" + type.pluginFormatName + ")", 5, 0, width, height, juce::Justification::centredLeft);
    }
}

int MainComponent::ActivePluginsModel::getNumRows() { return engine.getHostedPluginCount(); }
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

MainComponent::MainComponent(AudioEngine& engine)
    : audioEngine(engine),
      tabs(juce::TabbedButtonBar::TabsAtTop),
      graphicEq(engine.getFilterProcessor()),
      curveComponent(engine.getFilterProcessor()),
      deviceSelector(engine.getDeviceManager(), 2, 2, 2, 2, true, true, true, false),
      knownPluginsModel(engine),
      activePluginsModel(engine)
{
    setLookAndFeel(&tahoeLookAndFeel);
    startTimer(2000); // Check every 2 seconds
    
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

    addAndMakeVisible(resetEqButton);
    resetEqButton.onClick = [this] {
        graphicEq.resetGains();
    };

    addAndMakeVisible(globalBypassToggle);
    globalBypassToggle.onClick = [this] {
        audioEngine.getFilterProcessor().setBypassed(globalBypassToggle.getToggleState());
    };

    addAndMakeVisible(profileList);
    profileList.setTextWhenNoChoicesAvailable("No Profiles");

    // Layout Initializations
    eqLayout = std::make_unique<EqLayout>(graphicEq, curveComponent);
    pluginsLayout = std::make_unique<PluginsLayout>(*this);

    // Crossfeed / Preamp Config
    crossfeedToggle.setToggleState(false, juce::NotificationType::dontSendNotification);
    crossfeedToggle.onClick = [this] {
        if (auto node = audioEngine.getCrossfeedNode())
            if (auto* cf = dynamic_cast<equinox::CrossfeedProcessor*>(node->getProcessor()))
                cf->setEnabled(crossfeedToggle.getToggleState());
    };

    crossfeedAmount.setRange(0.0, 1.0, 0.01);
    crossfeedAmount.setValue(0.3);
    crossfeedAmount.onValueChange = [this] {
        if (auto node = audioEngine.getCrossfeedNode())
            if (auto* cf = dynamic_cast<equinox::CrossfeedProcessor*>(node->getProcessor()))
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
            if (audioEngine.addPlugin(list.getTypes()[row])) {
                activePluginsList.updateContent();
            }
        }
    };

    removeButton.onClick = [this] {
        auto row = activePluginsList.getSelectedRow();
        if (row >= 0) {
            audioEngine.removePlugin(row);
            activePluginWindow = nullptr;
            activePluginsList.updateContent();
        }
    };

    editButton.onClick = [this] {
        auto row = activePluginsList.getSelectedRow();
        if (row >= 0) {
            if (auto* p = audioEngine.getHostedPluginProcessor(row))
            {
                activePluginWindow = new PluginWindow(*p);
            }
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
    resetEqButton.setBounds(topBar.removeFromLeft(100).reduced(2));
    globalBypassToggle.setBounds(topBar.removeFromLeft(120).reduced(2));

    tabs.setBounds(area);
}

} // namespace equinox
