#include "PadGrid.h"

PadGrid::PadGrid (SamplerEngine& eng) : engine (eng)
{
    setMouseCursor (juce::MouseCursor::PointingHandCursor);

    for (int b = 0; b < SamplerEngine::NUM_BANKS; ++b)
    {
        bankButtons[b].setButtonText (engine.getBankName (b));
        bankButtons[b].onClick = [this, b]
        {
            engine.setCurrentBank (b);
            updateBankButtonColours();
            if (onBankChanged) onBankChanged (b);
            repaint();
        };
        addAndMakeVisible (bankButtons[b]);
    }
    updateBankButtonColours();
}

void PadGrid::paint (juce::Graphics& g)
{
    g.fillAll (Colours_::background);

    auto gridArea = getGridArea();
    int bank = engine.getCurrentBank();

    for (int i = 0; i < SamplerEngine::PADS_PER_BANK; ++i)
    {
        int absPad = engine.absolutePadIndex (bank, i);
        auto bounds = getPadBounds (i).toFloat();

        juce::Colour padColour = Colours_::padOff;
        if (absPad == selectedPad)
            padColour = Colours_::accent;
        else if (engine.hasSample (absPad))
            padColour = Colours_::surfaceLight;

        if (i == hoveredPad)
            padColour = padColour.brighter (0.15f);

        g.setColour (padColour);
        g.fillRoundedRectangle (bounds.reduced (2.0f), 6.0f);

        g.setColour (padColour.brighter (0.2f));
        g.drawRoundedRectangle (bounds.reduced (2.0f), 6.0f, 1.0f);

        // Pad number (bank-relative, 1-indexed)
        g.setColour (Colours_::textSecondary);
        g.setFont (11.0f);
        g.drawText (engine.getBankName (bank) + juce::String (i + 1),
                     bounds.reduced (6.0f), juce::Justification::topLeft);

        if (engine.hasSample (absPad))
        {
            g.setColour (Colours_::textPrimary);
            g.setFont (10.0f);
            auto name = engine.getPadState (absPad).sampleName;
            if (name.length() > 12) name = name.substring (0, 10) + "..";
            g.drawText (name, bounds.reduced (4.0f), juce::Justification::centred);
        }
        else
        {
            g.setColour (Colours_::textSecondary.withAlpha (0.4f));
            g.setFont (9.0f);
            g.drawText ("Drop\nSample", bounds.reduced (4.0f), juce::Justification::centred);
        }

        auto& state = engine.getPadState (absPad);
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

    if (dragOver)
    {
        g.setColour (Colours_::accent.withAlpha (0.2f));
        g.fillRect (gridArea);
        g.setColour (Colours_::accent);
        g.drawRect (gridArea, 2);
    }
}

void PadGrid::resized()
{
    auto area = getLocalBounds();

    // Bank buttons at the top
    auto bankBar = area.removeFromTop (28);
    int btnW = bankBar.getWidth() / SamplerEngine::NUM_BANKS;
    for (int b = 0; b < SamplerEngine::NUM_BANKS; ++b)
        bankButtons[b].setBounds (bankBar.removeFromLeft (btnW).reduced (2));
}

void PadGrid::mouseDown (const juce::MouseEvent& e)
{
    int localPad = getPadAtPosition (e.x, e.y);
    if (localPad >= 0)
    {
        int absPad = engine.absolutePadIndex (localPad);
        setSelectedPad (absPad);
        engine.triggerPad (absPad, 0.8f);
    }
}

void PadGrid::mouseUp (const juce::MouseEvent& e)
{
    int localPad = getPadAtPosition (e.x, e.y);
    if (localPad >= 0)
        engine.releasePad (engine.absolutePadIndex (localPad));
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
    int localPad = getPadAtPosition (x, y);
    if (localPad < 0) localPad = selectedPad - engine.getCurrentBank() * SamplerEngine::PADS_PER_BANK;
    if (localPad < 0) localPad = 0;

    for (auto& f : files)
    {
        if (localPad >= SamplerEngine::PADS_PER_BANK) break;

        int absPad = engine.absolutePadIndex (localPad);
        juce::File file (f);
        if (engine.loadSample (absPad, file))
        {
            if (onSampleLoaded)
                onSampleLoaded (absPad);
            localPad++;
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

void PadGrid::setSelectedPad (int absolutePadIndex)
{
    selectedPad = juce::jlimit (0, SamplerEngine::TOTAL_PADS - 1, absolutePadIndex);

    // Switch bank if the selected pad is in a different bank
    int bank = selectedPad / SamplerEngine::PADS_PER_BANK;
    if (bank != engine.getCurrentBank())
    {
        engine.setCurrentBank (bank);
        updateBankButtonColours();
    }

    if (onPadSelected)
        onPadSelected (selectedPad);
    repaint();
}

int PadGrid::getPadAtPosition (int x, int y) const
{
    for (int i = 0; i < SamplerEngine::PADS_PER_BANK; ++i)
    {
        if (getPadBounds (i).contains (x, y))
            return i;
    }
    return -1;
}

juce::Rectangle<int> PadGrid::getGridArea() const
{
    auto area = getLocalBounds();
    area.removeFromTop (28); // bank buttons
    return area;
}

juce::Rectangle<int> PadGrid::getPadBounds (int padIndex) const
{
    auto gridArea = getGridArea();
    int cols = 4;
    int rows = 4;
    int col = padIndex % cols;
    int row = rows - 1 - (padIndex / cols);

    int padW = gridArea.getWidth() / cols;
    int padH = gridArea.getHeight() / rows;

    return { gridArea.getX() + col * padW, gridArea.getY() + row * padH, padW, padH };
}

void PadGrid::updateBankButtonColours()
{
    int current = engine.getCurrentBank();
    for (int b = 0; b < SamplerEngine::NUM_BANKS; ++b)
    {
        if (b == current)
        {
            bankButtons[b].setColour (juce::TextButton::buttonColourId, Colours_::accent);
            bankButtons[b].setColour (juce::TextButton::textColourOffId, Colours_::textPrimary);
        }
        else
        {
            bankButtons[b].setColour (juce::TextButton::buttonColourId, Colours_::surface);
            bankButtons[b].setColour (juce::TextButton::textColourOffId, Colours_::textSecondary);
        }
    }
}
