#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include "../dsp/AudioEngine.h"

namespace equinox
{

class MenuBarIcon : public juce::SystemTrayIconComponent
{
public:
    MenuBarIcon(AudioEngine& engine, std::function<void()> openWindowCallback);
    ~MenuBarIcon() override = default;

    void mouseDown(const juce::MouseEvent& e) override;

private:
    AudioEngine& m_audioEngine;
    std::function<void()> m_openWindowCallback;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MenuBarIcon)
};

} // namespace equinox
