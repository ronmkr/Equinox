#include "MenuBarIcon.h"

namespace equinox
{

MenuBarIcon::MenuBarIcon(AudioEngine& engine, std::function<void()> openWindowCallback)
    : m_audioEngine(engine),
      m_openWindowCallback(openWindowCallback)
{
    // Create a beautiful sine wave template icon
    juce::Image icon(juce::Image::ARGB, 22, 22, true);
    juce::Graphics g(icon);
    
    g.setColour(juce::Colours::black);
    juce::Path p;
    auto w = 22.0f;
    auto h = 22.0f;
    
    p.startNewSubPath(2.0f, h * 0.5f);
    for (float x = 2.0f; x <= w - 2.0f; x += 1.0f)
    {
        float y = h * 0.5f + std::sin((x / (w - 4.0f)) * juce::MathConstants<float>::twoPi) * (h * 0.3f);
        p.lineTo(x, y);
    }
    
    g.strokePath(p, juce::PathStrokeType(2.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    
    setIconImage(icon, icon); // Template icon
    setIconTooltip("Equinox Audiophile EQ");
}

void MenuBarIcon::mouseDown(const juce::MouseEvent& e)
{
    juce::PopupMenu m;
    
    auto& fp = m_audioEngine.getFilterProcessor();
    
    m.addSectionHeader("Equinox DSP");
    m.addItem(1, "Open Equalizer...", true, false);
    m.addSeparator();

    m.addItem(2, "Bypass EQ", true, fp.isBypassed());
    m.addItem(3, "Toggle A/B", true, false);

    m.addSeparator();
    m.addItem(10, "Quit", true, false);

    m.showMenuAsync(juce::PopupMenu::Options().withTargetScreenArea(getScreenBounds()),
        [this, &fp](int result)
        {
            if (result == 1)
            {
                if (m_openWindowCallback) m_openWindowCallback();
            }
            else if (result == 2)
            {
                fp.setBypassed(!fp.isBypassed());
            }
            else if (result == 3)
            {
                fp.toggleAB();
            }
            else if (result == 10)
            {
                juce::JUCEApplication::getInstance()->systemRequestedQuit();
            }
        });

}

} // namespace equinox
