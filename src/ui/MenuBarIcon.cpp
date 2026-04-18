#include "MenuBarIcon.h"

namespace equinox
{

MenuBarIcon::MenuBarIcon(AudioEngine& engine, std::function<void()> openWindowCallback)
    : m_audioEngine(engine),
      m_openWindowCallback(openWindowCallback)
{
    // For now, we use a simple built-in icon or a solid color square 
    // because we don't have BinaryData assets yet.
    // In a real app, you'd use a template PNG icon.
    juce::Image icon(juce::Image::ARGB, 20, 20, true);
    juce::Graphics g(icon);
    g.setColour(juce::Colours::white);
    g.fillEllipse(2, 2, 16, 16);
    
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
    
    m.addItem(2, "Bypass EQ", true, false); // Toggle logic would go here
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
