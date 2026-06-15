#pragma once
#include <JuceHeader.h>

namespace Colours_
{
    // Dark theme palette
    const juce::Colour background     { 0xff1a1a2e };
    const juce::Colour surface        { 0xff16213e };
    const juce::Colour surfaceLight   { 0xff0f3460 };
    const juce::Colour accent         { 0xffe94560 };
    const juce::Colour accentAlt      { 0xff533483 };
    const juce::Colour textPrimary    { 0xfff0f0f0 };
    const juce::Colour textSecondary  { 0xff8a8a9a };
    const juce::Colour padOff         { 0xff2a2a4a };
    const juce::Colour padOn          { 0xffe94560 };
    const juce::Colour padHover       { 0xff3a3a5e };
    const juce::Colour stepActive     { 0xff00d2ff };
    const juce::Colour stepCurrent    { 0xffffd700 };
    const juce::Colour knobTrack      { 0xff3a3a5e };
    const juce::Colour knobFill       { 0xff00d2ff };
    const juce::Colour meterGreen     { 0xff00e676 };
    const juce::Colour meterYellow    { 0xffffea00 };
    const juce::Colour meterRed       { 0xffff1744 };
    const juce::Colour waveform       { 0xff00d2ff };
    const juce::Colour waveformBg     { 0xff0d1117 };
    const juce::Colour muteColour     { 0xffff6b35 };
    const juce::Colour soloColour     { 0xffffd700 };
}

class CustomLookAndFeel : public juce::LookAndFeel_V4
{
public:
    CustomLookAndFeel();

    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPosProportional, float rotaryStartAngle,
                           float rotaryEndAngle, juce::Slider& slider) override;

    void drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPos, float minSliderPos, float maxSliderPos,
                           juce::Slider::SliderStyle style, juce::Slider& slider) override;

    void drawButtonBackground (juce::Graphics& g, juce::Button& button,
                               const juce::Colour& backgroundColour,
                               bool shouldDrawButtonAsHighlighted,
                               bool shouldDrawButtonAsDown) override;

    void drawButtonText (juce::Graphics& g, juce::TextButton& button,
                         bool shouldDrawButtonAsHighlighted,
                         bool shouldDrawButtonAsDown) override;

    juce::Font getTextButtonFont (juce::TextButton&, int buttonHeight) override;

    void drawComboBox (juce::Graphics& g, int width, int height, bool isButtonDown,
                       int buttonX, int buttonY, int buttonW, int buttonH,
                       juce::ComboBox& box) override;

    void drawLabel (juce::Graphics& g, juce::Label& label) override;
};
