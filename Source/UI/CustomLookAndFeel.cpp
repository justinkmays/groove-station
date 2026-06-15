#include "CustomLookAndFeel.h"

CustomLookAndFeel::CustomLookAndFeel()
{
    setColour (juce::ResizableWindow::backgroundColourId, Colours_::background);
    setColour (juce::TextButton::buttonColourId, Colours_::surface);
    setColour (juce::TextButton::textColourOffId, Colours_::textPrimary);
    setColour (juce::TextButton::textColourOnId, Colours_::textPrimary);
    setColour (juce::ComboBox::backgroundColourId, Colours_::surface);
    setColour (juce::ComboBox::textColourId, Colours_::textPrimary);
    setColour (juce::ComboBox::outlineColourId, Colours_::surfaceLight);
    setColour (juce::PopupMenu::backgroundColourId, Colours_::surface);
    setColour (juce::PopupMenu::textColourId, Colours_::textPrimary);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, Colours_::accent);
    setColour (juce::Label::textColourId, Colours_::textPrimary);
    setColour (juce::Slider::textBoxTextColourId, Colours_::textPrimary);
    setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setColour (juce::ScrollBar::thumbColourId, Colours_::surfaceLight);
}

void CustomLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                           float sliderPos, float rotaryStartAngle,
                                           float rotaryEndAngle, juce::Slider&)
{
    float radius = (float) juce::jmin (width / 2, height / 2) - 4.0f;
    float centreX = (float) x + (float) width * 0.5f;
    float centreY = (float) y + (float) height * 0.5f;
    float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    // Background track
    juce::Path bgArc;
    bgArc.addCentredArc (centreX, centreY, radius, radius, 0.0f,
                          rotaryStartAngle, rotaryEndAngle, true);
    g.setColour (Colours_::knobTrack);
    g.strokePath (bgArc, juce::PathStrokeType (3.0f, juce::PathStrokeType::curved,
                                                juce::PathStrokeType::rounded));

    // Value arc
    juce::Path valueArc;
    valueArc.addCentredArc (centreX, centreY, radius, radius, 0.0f,
                             rotaryStartAngle, angle, true);
    g.setColour (Colours_::knobFill);
    g.strokePath (valueArc, juce::PathStrokeType (3.0f, juce::PathStrokeType::curved,
                                                   juce::PathStrokeType::rounded));

    // Knob body
    float innerRadius = radius * 0.65f;
    g.setColour (Colours_::surface);
    g.fillEllipse (centreX - innerRadius, centreY - innerRadius,
                   innerRadius * 2.0f, innerRadius * 2.0f);

    g.setColour (Colours_::surfaceLight);
    g.drawEllipse (centreX - innerRadius, centreY - innerRadius,
                   innerRadius * 2.0f, innerRadius * 2.0f, 1.0f);

    // Pointer
    juce::Path pointer;
    float pointerLength = innerRadius * 0.7f;
    float pointerThickness = 2.0f;
    pointer.addRoundedRectangle (-pointerThickness * 0.5f, -pointerLength, pointerThickness, pointerLength, 1.0f);
    pointer.applyTransform (juce::AffineTransform::rotation (angle).translated (centreX, centreY));
    g.setColour (Colours_::knobFill);
    g.fillPath (pointer);
}

void CustomLookAndFeel::drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height,
                                           float sliderPos, float /*minSliderPos*/, float /*maxSliderPos*/,
                                           juce::Slider::SliderStyle style, juce::Slider&)
{
    bool isVertical = (style == juce::Slider::LinearVertical || style == juce::Slider::LinearBarVertical);

    if (isVertical)
    {
        float trackX = (float) x + (float) width * 0.5f;
        float trackTop = (float) y + 4.0f;
        float trackBottom = (float) y + (float) height - 4.0f;

        // Track background
        g.setColour (Colours_::knobTrack);
        g.fillRoundedRectangle (trackX - 2.0f, trackTop, 4.0f, trackBottom - trackTop, 2.0f);

        // Filled portion
        g.setColour (Colours_::knobFill);
        g.fillRoundedRectangle (trackX - 2.0f, sliderPos, 4.0f, trackBottom - sliderPos, 2.0f);

        // Thumb
        g.setColour (Colours_::textPrimary);
        g.fillRoundedRectangle (trackX - 8.0f, sliderPos - 4.0f, 16.0f, 8.0f, 3.0f);
    }
    else
    {
        float trackY = (float) y + (float) height * 0.5f;
        float trackLeft = (float) x + 4.0f;
        float trackRight = (float) x + (float) width - 4.0f;

        g.setColour (Colours_::knobTrack);
        g.fillRoundedRectangle (trackLeft, trackY - 2.0f, trackRight - trackLeft, 4.0f, 2.0f);

        g.setColour (Colours_::knobFill);
        g.fillRoundedRectangle (trackLeft, trackY - 2.0f, sliderPos - trackLeft, 4.0f, 2.0f);

        g.setColour (Colours_::textPrimary);
        g.fillEllipse (sliderPos - 6.0f, trackY - 6.0f, 12.0f, 12.0f);
    }
}

void CustomLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& button,
                                               const juce::Colour& backgroundColour,
                                               bool shouldDrawButtonAsHighlighted,
                                               bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds().toFloat().reduced (0.5f);
    auto baseColour = backgroundColour;

    if (shouldDrawButtonAsDown)
        baseColour = baseColour.brighter (0.2f);
    else if (shouldDrawButtonAsHighlighted)
        baseColour = baseColour.brighter (0.1f);

    if (button.getToggleState())
        baseColour = Colours_::accent;

    g.setColour (baseColour);
    g.fillRoundedRectangle (bounds, 4.0f);

    g.setColour (baseColour.brighter (0.1f));
    g.drawRoundedRectangle (bounds, 4.0f, 1.0f);
}

void CustomLookAndFeel::drawButtonText (juce::Graphics& g, juce::TextButton& button,
                                         bool, bool)
{
    auto font = getTextButtonFont (button, button.getHeight());
    g.setFont (font);

    g.setColour (button.getToggleState() ? Colours_::textPrimary : button.findColour (juce::TextButton::textColourOffId));
    g.drawText (button.getButtonText(), button.getLocalBounds(),
                juce::Justification::centred, true);
}

juce::Font CustomLookAndFeel::getTextButtonFont (juce::TextButton&, int buttonHeight)
{
    return juce::Font (juce::jmin (14.0f, (float) buttonHeight * 0.55f));
}

void CustomLookAndFeel::drawComboBox (juce::Graphics& g, int width, int height, bool,
                                       int, int, int, int, juce::ComboBox& box)
{
    auto bounds = juce::Rectangle<int> (0, 0, width, height).toFloat().reduced (0.5f);
    g.setColour (box.findColour (juce::ComboBox::backgroundColourId));
    g.fillRoundedRectangle (bounds, 4.0f);
    g.setColour (box.findColour (juce::ComboBox::outlineColourId));
    g.drawRoundedRectangle (bounds, 4.0f, 1.0f);

    // Arrow
    float arrowX = (float) width - 20.0f;
    float arrowY = (float) height * 0.5f;
    juce::Path arrow;
    arrow.addTriangle (arrowX - 4.0f, arrowY - 2.0f, arrowX + 4.0f, arrowY - 2.0f, arrowX, arrowY + 4.0f);
    g.setColour (Colours_::textSecondary);
    g.fillPath (arrow);
}

void CustomLookAndFeel::drawLabel (juce::Graphics& g, juce::Label& label)
{
    g.fillAll (label.findColour (juce::Label::backgroundColourId));

    auto textArea = label.getBorderSize().subtractedFrom (label.getLocalBounds());
    g.setColour (label.findColour (juce::Label::textColourId));
    g.setFont (label.getFont());
    g.drawText (label.getText(), textArea, label.getJustificationType(), true);
}
