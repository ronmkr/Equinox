#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <vector>

namespace equinox
{

class OscilloscopeComponent : public juce::Component, public juce::Timer
{
public:
    OscilloscopeComponent() { startTimer(30); }
    
    void pushSample(float sample) {
        m_buffer[m_writePos] = sample;
        m_writePos = (m_writePos + 1) % m_buffer.size();
    }

    void paint(juce::Graphics& g) override {
        g.fillAll(juce::Colours::black);
        g.setColour(juce::Colours::green);
        
        juce::Path p;
        auto w = (float)getWidth();
        auto h = (float)getHeight();
        auto midY = h * 0.5f;
        
        p.startNewSubPath(0, midY);
        
        for (size_t x = 0; x < m_buffer.size(); ++x) {
            int readPos = (m_writePos + x) % m_buffer.size();
            float val = m_buffer[readPos];
            float xPos = (float)x / m_buffer.size() * w;
            float yPos = midY - (val * midY * 0.8f);
            p.lineTo(xPos, yPos);
        }
        
        g.strokePath(p, juce::PathStrokeType(1.5f));
    }

    void timerCallback() override { repaint(); }

private:
    std::vector<float> m_buffer { 512, 0.0f };
    int m_writePos = 0;
};

} // namespace equinox
