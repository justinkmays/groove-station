#include "PadGrid.h"

PadGrid::PadGrid (SamplerEngine& eng) : engine (eng)
{
    setMouseCursor (juce::MouseCursor::PointingHandCursor);
}

void PadGrid::paint (juce::Graphics& g)
{
    g.fillAll (Colours_::background);

    for (int i = 0; i < SamplerEngine::NUM_PADS; ++i)
    {
        auto bounds = getPadBounds (i).toFloat();

        // Pad colour
        juce::Colour padColour = Colours_::padOff;
        if (i == selectedPad)
            padColour = Colours_::accent;
        else if (engine.hasSample (i))
            padColour = Colours_::surfaceLight;

        if (i == hoveredPad)
            padColour = padColour.brighter (0.15f);

        // Draw pad with rounded corners
        g.setColour (padColour);
        g.fillRoundedRectangle (bounds.reduced (2.0f), 6.0f);

        // Border
        g.setColour (padColour.brighter (0.2f));
        g.drawRoundedRectangle (bounds.reduced (2.0f), 6.0f, 1.0f);

        // Pad number
        g.setColour (Colours_::textSecondary);
        g.setFont (11.0f);
        g.drawText (juce::String (i + 1), bounds.reduced (6.0f),
                     juce::Justification::topLeft);

        // Sample name
        if (engine.hasSample (i))
        {
            g.setColour (Colours_::textPrimary);
            g.setFont (10.0f);
            auto name = engine.getPadState (i).sampleName;
            if (name.length() > 12) name = name.substring (0, 10) + "..";
            g.drawText (name, bounds.reduced (4.0f),
                         juce::Justification::centred);
        }
        else
        {
            g.setColour (Colours_::textSecondary.withAlpha (0.4f));
            g.setFont (9.0f);
            g.drawText ("Drop\nSample", bounds.reduced (4.0f),
                         juce::Justification::centred);
        }

        // Mute/Solo indicators
        auto& state = engine.getPadState (i);
        if (state.mute)
        {
            g.setColour (Colours_::muteColour);
            g.fillEllipse (bounds.getRight() - 14.0f, bounds.getY() + 4.0f, 8.0f, 8.0f);
        }
        if (state.solo)
        {
            g.setColour (Colours_::soloColour);
            g.fillEllipse (bounds.getRight() - 14.0f, bounds.getY() + 14.0f, 8.0f, 8.0f);
        }
    }

    // Drag-over overlay
    if (dragOver)
    {
        g.setColour (Colours_::accent.withAlpha (0.2f));
        g.fillAll();
        g.setColour (Colours_::accent);
        g.drawRect (getLocalBounds(), 2);
    }
}

void PadGrid::resized()
{
}

void PadGrid::mouseDown (const juce::MouseEvent& e)
{
    int pad = getPadAtPosition (e.x, e.y);
    if (pad >= 0)
    {
        setSelectedPad (pad);
        engine.triggerPad (pad, 0.8f);
    }
}

void PadGrid::mouseUp (const juce::MouseEvent& e)
{
    int pad = getPadAtPosition (e.x, e.y);
    if (pad >= 0)
        engine.releasePad (pad);
}

bool PadGrid::isInterestedInFileDrag (const juce::StringArray& files)
{
    for (auto& f : files)
    {
        auto ext = juce::File (f).getFileExtension().toLowerCase();
        if (ext == ".wav" || ext == ".aiff" || ext == ".aif" || ext == ".mp3"
            || ext == ".flac" || ext == ".ogg")
            return true;
    }
    return false;
}

void PadGrid::filesDropped (const juce::StringArray& files, int x, int y)
{
    dragOver = false;
    int pad = getPadAtPosition (x, y);
    if (pad < 0) pad = selectedPad;

    for (auto& f : files)
    {
        if (pad >= SamplerEngine::NUM_PADS) break;

        juce::File file (f);
        if (engine.loadSample (pad, file))
        {
            if (onSampleLoaded)
                onSampleLoaded (pad);
            pad++;
        }
    }
    repaint();
}

void PadGrid::fileDragEnter (const juce::StringArray&, int, int)
{
    dragOver = true;
    repaint();
}

void PadGrid::fileDragExit (const juce::StringArray&)
{
    dragOver = false;
    repaint();
}

void PadGrid::setSelectedPad (int pad)
{
    selectedPad = juce::jlimit (0, SamplerEngine::NUM_PADS - 1, pad);
    if (onPadSelected)
        onPadSelected (selectedPad);
    repaint();
}

int PadGrid::getPadAtPosition (int x, int y) const
{
    for (int i = 0; i < SamplerEngine::NUM_PADS; ++i)
    {
        if (getPadBounds (i).contains (x, y))
            return i;
    }
    return -1;
}

juce::Rectangle<int> PadGrid::getPadBounds (int padIndex) const
{
    int cols = 4;
    int rows = 4;
    int col = padIndex % cols;
    // Layout bottom-to-top like MPC (pad 0 = bottom-left)
    int row = rows - 1 - (padIndex / cols);

    int padW = getWidth() / cols;
    int padH = getHeight() / rows;

    return { col * padW, row * padH, padW, padH };
}
