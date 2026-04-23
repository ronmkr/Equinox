#include "MainComponent.h"

namespace equinox
{

MainComponent::MainComponent(AudioEngine& engine)
    : header(inputMeter, outputMeter),
      m_audioEngine(engine),
      curveComponent(engine.getFilterProcessor(), engine),
      graphicEq(engine.getFilterProcessor()),
      deviceSelector(engine.getDeviceManager(), 2, 2, 2, 2, true, true, true, false)
{
    setLookAndFeel(&m_tahoeLookAndFeel);
    startTimer(30);
    
    // --- Rack components ---
    addAndMakeVisible(header);
    
    addAndMakeVisible(inputStrip);
    addAndMakeVisible(oscilloscope);
    addAndMakeVisible(inputMeter);
    
    addAndMakeVisible(eqStrip);
    addAndMakeVisible(curveComponent);
    addAndMakeVisible(graphicEq);
    
    addAndMakeVisible(masterStrip);
    addAndMakeVisible(masterFader);
    addAndMakeVisible(outputMeter);
    
    addAndMakeVisible(crossfeedToggle);
    addAndMakeVisible(compressorToggle);
    addAndMakeVisible(loudnessToggle);
    addAndMakeVisible(convolutionToggle);
    addAndMakeVisible(bypassToggle);
    addAndMakeVisible(hd600Button);
    addAndMakeVisible(loadIrButton);
    addAndMakeVisible(mapDeviceButton);
    addAndMakeVisible(resetBtn);

    masterFader.setRange(-60.0, 12.0, 0.1);
    masterFader.setValue(0.0);
    masterFader.onValueChange = [this] {
        m_audioEngine.getFilterProcessor().setPreamp((float)masterFader.getValue());
    };

    addChildComponent(deviceSelector);
    header.settingsButton.onClick = [this] {
        m_isSettingsOpen = !m_isSettingsOpen;
        deviceSelector.setVisible(m_isSettingsOpen);
        resized();
    };

    // --- Logic ---
    bypassToggle.setToggleState(m_audioEngine.getFilterProcessor().isBypassed(), juce::dontSendNotification);
    bypassToggle.onClick = [this] {
        m_audioEngine.getFilterProcessor().setBypassed(bypassToggle.getToggleState());
    };

    crossfeedToggle.onClick = [this] {
        m_audioEngine.getCrossfeedProcessor().setEnabled(crossfeedToggle.getToggleState());
    };

    compressorToggle.setToggleState(false, juce::dontSendNotification); // Default off
    compressorToggle.onClick = [this] {
        m_audioEngine.setCompressorEnabled(compressorToggle.getToggleState());
    };

    loudnessToggle.setToggleState(m_audioEngine.getLoudnessProcessor().isEnabled(), juce::dontSendNotification);
    loudnessToggle.onClick = [this] {
        m_audioEngine.setLoudnessEnabled(loudnessToggle.getToggleState());
    };

    convolutionToggle.setToggleState(m_audioEngine.getConvolutionProcessor().isEnabled(), juce::dontSendNotification);
    convolutionToggle.onClick = [this] {
        m_audioEngine.setConvolutionEnabled(convolutionToggle.getToggleState());
    };

    loadIrButton.onClick = [this] {
        m_irChooser = std::make_unique<juce::FileChooser> ("Select an Impulse Response...", 
                                                           juce::File::getSpecialLocation(juce::File::userHomeDirectory),
                                                           "*.wav;*.aif;*.aiff");
        m_irChooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                                 [this] (const juce::FileChooser& chooser)
                                 {
                                     auto file = chooser.getResult();
                                     if (file.existsAsFile())
                                     {
                                         m_audioEngine.loadImpulseResponse(file);
                                         convolutionToggle.setToggleState(true, juce::sendNotification);
                                     }
                                 });
    };

    mapDeviceButton.onClick = [this] {
        m_audioEngine.saveCurrentProfileAs("HD 600");
        m_audioEngine.mapCurrentDeviceToProfile("HD 600");
    };

    resetBtn.onClick = [this] { graphicEq.resetGains(); masterFader.setValue(0.0); };
    
    hd600Button.onClick = [this] {
        struct FilterParam { float f; float g; float q; };
        std::vector<FilterParam> params = {
            { 25, 6.3, 0.87 }, { 185, -1.9, 1.22 }, { 618, 1.0, 1.42 },
            { 3143, -2.4, 4.78 }, { 4332, 2.9, 6.80 }, { 5979, -4.2, 6.09 },
            { 6374, 2.8, 3.32 }, { 9346, 4.2, 2.41 }, { 11893, 2.3, 1.55 },
            { 20166, -10.7, 0.24 }
        };
        m_audioEngine.getFilterProcessor().setPreamp(-6.8f);
        masterFader.setValue(-6.8);
        for (int i = 0; i < (int)params.size(); ++i)
            m_audioEngine.getFilterProcessor().updateBandGain(i, params[i].f, params[i].g, params[i].q);
    };

    setSize(1000, 600);

    // Safety: Connect callback LAST
    juce::Component::SafePointer<MainComponent> safeThis(this);
    m_audioEngine.onSampleReceived = [safeThis](float sample) {
        if (safeThis != nullptr)
            safeThis->oscilloscope.pushSample(sample);
    };
}

MainComponent::~MainComponent()
{
    m_audioEngine.onSampleReceived = nullptr;
    setLookAndFeel(nullptr);
}

void MainComponent::timerCallback()
{
    m_tahoeLookAndFeel.refreshColours();
    m_inputLevel = m_audioEngine.getInputLevel();
    m_outputLevel = m_audioEngine.getOutputLevel();
    repaint();
}

void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void MainComponent::resized()
{
    auto area = getLocalBounds();
    header.setBounds(area.removeFromTop(60));
    
    if (m_isSettingsOpen) {
        deviceSelector.setBounds(area.removeFromBottom(200).reduced(10));
    }

    auto main = area.reduced(10);
    
    // INPUT STRIP
    auto left = main.removeFromLeft(200);
    inputStrip.setBounds(left);
    auto leftInner = left.reduced(10, 30);
    oscilloscope.setBounds(leftInner.removeFromTop(150));
    inputMeter.setBounds(leftInner.removeFromTop(30).reduced(10, 5));
    
    // DSP RACK
    auto mid = main.removeFromLeft(600);
    eqStrip.setBounds(mid);
    auto midInner = mid.reduced(10, 30);
    curveComponent.setBounds(midInner.removeFromTop(200));
    graphicEq.setBounds(midInner.removeFromTop(200));
    
    // MASTER STRIP
    auto right = main;
    masterStrip.setBounds(right);
    auto rightInner = right.reduced(10, 30);
    masterFader.setBounds(rightInner.removeFromLeft(60));
    outputMeter.setBounds(rightInner.removeFromLeft(30).reduced(0, 5));
    
    auto tools = rightInner;
    bypassToggle.setBounds(tools.removeFromTop(30));
    crossfeedToggle.setBounds(tools.removeFromTop(30));
    compressorToggle.setBounds(tools.removeFromTop(30));
    loudnessToggle.setBounds(tools.removeFromTop(30));
    convolutionToggle.setBounds(tools.removeFromTop(30));
    resetBtn.setBounds(tools.removeFromBottom(40).reduced(2));
    hd600Button.setBounds(tools.removeFromBottom(40).reduced(2));
    loadIrButton.setBounds(tools.removeFromBottom(40).reduced(2));
    mapDeviceButton.setBounds(tools.removeFromBottom(40).reduced(2));
}

} // namespace equinox
