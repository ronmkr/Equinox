#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace equinox
{

/**
 * @class TahoeLookAndFeel
 * @brief A modern, vibrant macOS-inspired look and feel that adapts to system theme.
 */
class TahoeLookAndFeel : public juce::LookAndFeel_V4
{
public:
    TahoeLookAndFeel()
    {
        refreshColours();
    }

    void refreshColours()
    {
        // Portable way to check if current colour scheme is dark
        auto scheme = getCurrentColourScheme();
        auto isDark = scheme.getUIColour(ColourScheme::UIColour::windowBackground).getBrightness() < 0.5f;
        
        if (isDark)
        {
            setColour(juce::ResizableWindow::backgroundColourId, juce::Colour(0xff1a1a1a));
            setColour(juce::TextButton::buttonColourId, juce::Colour(0xff333333));
            setColour(juce::TextButton::textColourOffId, juce::Colours::white);
            setColour(juce::Slider::thumbColourId, juce::Colours::white);
            setColour(juce::Slider::trackColourId, juce::Colour(0xff444444));
            setColour(juce::Slider::backgroundColourId, juce::Colour(0xff222222));
            m_accentColour = juce::Colours::cyan;
        }
        else
        {
            setColour(juce::ResizableWindow::backgroundColourId, juce::Colour(0xfff0f0f0));
            setColour(juce::TextButton::buttonColourId, juce::Colour(0xffe0e0e0));
            setColour(juce::TextButton::textColourOffId, juce::Colours::black);
            setColour(juce::Slider::thumbColourId, juce::Colour(0xff333333));
            setColour(juce::Slider::trackColourId, juce::Colour(0xffcccccc));
            setColour(juce::Slider::backgroundColourId, juce::Colour(0xffdddddd));
            m_accentColour = juce::Colour(0xff007aff); // macOS Blue
        }
        
        setColour(juce::TabbedButtonBar::tabOutlineColourId, juce::Colours::transparentBlack);
        setColour(juce::TabbedButtonBar::frontOutlineColourId, juce::Colours::transparentBlack);
    }

    void drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour,
                              bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        auto bounds = button.getLocalBounds().toFloat().reduced(0.5f);
        auto baseColour = backgroundColour.withMultipliedSaturation(shouldDrawButtonAsHighlighted ? 1.3f : 1.0f)
                                           .withMultipliedBrightness(shouldDrawButtonAsDown ? 0.7f : 1.0f);

        if (shouldDrawButtonAsHighlighted) baseColour = baseColour.brighter(0.1f);

        g.setColour(baseColour);
        g.fillRoundedRectangle(bounds, 6.0f);
        
        g.setColour(findColour(juce::TextButton::textColourOffId).withAlpha(0.1f));
        g.drawRoundedRectangle(bounds, 6.0f, 1.0f);
    }

    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPos, float minSliderPos, float maxSliderPos,
                          const juce::Slider::SliderStyle style, juce::Slider& slider) override
    {
        auto isVertical = style == juce::Slider::LinearVertical;
        auto trackWidth = 4.0f;

        if (isVertical)
        {
            auto trackX = x + (width - trackWidth) * 0.5f;
            
            // Track background
            g.setColour(slider.findColour(juce::Slider::backgroundColourId));
            g.fillRoundedRectangle(trackX, (float)y, trackWidth, (float)height, trackWidth * 0.5f);

            // Active track (from middle 0dB)
            auto midY = (float)y + height * 0.5f;
            g.setColour(m_accentColour.withAlpha(0.8f));
            if (sliderPos < midY)
                g.fillRoundedRectangle(trackX, sliderPos, trackWidth, midY - sliderPos, trackWidth * 0.5f);
            else
                g.fillRoundedRectangle(trackX, midY, trackWidth, sliderPos - midY, trackWidth * 0.5f);

            // Thumb
            g.setColour(slider.findColour(juce::Slider::thumbColourId));
            g.fillEllipse(x + (width - 12.0f) * 0.5f, sliderPos - 6.0f, 12.0f, 12.0f);
        }
        else
        {
            LookAndFeel_V4::drawLinearSlider(g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, slider);
        }
    }

    juce::Font getTabButtonFont(juce::TabBarButton&, float height) override
    {
        return juce::FontOptions("San Francisco", height * 0.45f, juce::Font::plain);
    }

private:
    juce::Colour m_accentColour;
};

} // namespace equinox
