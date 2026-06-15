#include "SequencerGrid.h"

SequencerGrid::SequencerGrid (StepSequencer& seq, SamplerEngine& eng)
    : sequencer (seq), engine (eng)
{
    setupControls();
    startTimerHz (30);
}

SequencerGrid::~SequencerGrid()
{
    stopTimer();
}

void SequencerGrid::setupControls()
{
    addAndMakeVisible (patternSelector);
    for (int i = 0; i < StepSequencer::MAX_PATTERNS; ++i)
        patternSelector.addItem ("Pattern " + juce::String (i + 1), i + 1);
    patternSelector.setSelectedId (1);
    patternSelector.onChange = [this]
    {
        sequencer.setCurrentPattern (patternSelector.getSelectedId() - 1);
        repaint();
    };

    addAndMakeVisible (stepsSlider);
    stepsSlider.setRange (1, 64, 1);
    stepsSlider.setValue (16);
    stepsSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    stepsSlider.setTextBoxStyle (juce::Slider::TextBoxRight, false, 30, 20);
    stepsSlider.onValueChange = [this]
    {
        sequencer.getCurrentPattern().setNumSteps ((int) stepsSlider.getValue());
        repaint();
    };

    addAndMakeVisible (clearPatternButton);
    clearPatternButton.onClick = [this]
    {
        sequencer.clearPattern (sequencer.getCurrentPatternIndex());
        repaint();
    };

    addAndMakeVisible (copyPatternButton);
    copyPatternButton.onClick = [this]
    {
        int next = (sequencer.getCurrentPatternIndex() + 1) % StepSequencer::MAX_PATTERNS;
        sequencer.copyPattern (sequencer.getCurrentPatternIndex(), next);
    };

    addAndMakeVisible (viewToggle);
    viewToggle.setClickingTogglesState (true);
    viewToggle.onClick = [this]
    {
        showAllPads = ! viewToggle.getToggleState();
        viewToggle.setButtonText (showAllPads ? "ALL PADS" : "SELECTED");
        repaint();
    };
}

void SequencerGrid::paint (juce::Graphics& g)
{
    g.fillAll (Colours_::background);

    auto& pattern = sequencer.getCurrentPattern();
    int numSteps = pattern.getNumSteps();
    int currentStep = sequencer.getCurrentStep();
    bool isPlaying = sequencer.isPlaying();

    int startPad = showAllPads ? 0 : selectedPad;
    int endPad = showAllPads ? SamplerEngine::NUM_PADS : selectedPad + 1;

    for (int pad = startPad; pad < endPad; ++pad)
    {
        for (int step = 0; step < numSteps; ++step)
        {
            auto bounds = getStepBounds (pad, step).toFloat();
            auto& stepData = pattern.getStep (pad, step);

            juce::Colour colour = Colours_::padOff;

            if (stepData.active)
            {
                colour = Colours_::stepActive;
                // Velocity visualization: brightness
                float alpha = 0.4f + stepData.velocity * 0.6f;
                colour = colour.withAlpha (alpha);
            }

            // Current step highlight
            if (isPlaying && step == currentStep)
            {
                colour = stepData.active ? Colours_::stepCurrent : Colours_::stepCurrent.withAlpha (0.3f);
            }

            // Beat markers (every 4 steps)
            if (step % 4 == 0 && ! stepData.active && ! (isPlaying && step == currentStep))
            {
                colour = colour.brighter (0.05f);
            }

            g.setColour (colour);
            g.fillRoundedRectangle (bounds.reduced (1.0f), 2.0f);

            // Border
            g.setColour (colour.brighter (0.15f));
            g.drawRoundedRectangle (bounds.reduced (1.0f), 2.0f, 0.5f);

            // Probability indicator (if < 1.0)
            if (stepData.active && stepData.probability < 0.99f)
            {
                g.setColour (Colours_::textSecondary);
                g.setFont (7.0f);
                g.drawText (juce::String ((int)(stepData.probability * 100)) + "%",
                             bounds.toNearestInt(), juce::Justification::centred);
            }
        }

        // Pad label
        if (showAllPads)
        {
            g.setColour (pad == selectedPad ? Colours_::accent : Colours_::textSecondary);
            g.setFont (9.0f);
            auto labelBounds = getStepBounds (pad, 0);
            g.drawText (juce::String (pad + 1), labelBounds.getX() - 20, labelBounds.getY(),
                        18, labelBounds.getHeight(), juce::Justification::centredRight);
        }
    }
}

void SequencerGrid::resized()
{
    auto area = getLocalBounds();
    auto controls = area.removeFromTop (30);

    patternSelector.setBounds (controls.removeFromLeft (120).reduced (2));
    controls.removeFromLeft (8);

    auto stepsArea = controls.removeFromLeft (120);
    stepsSlider.setBounds (stepsArea.reduced (2));

    controls.removeFromLeft (4);
    clearPatternButton.setBounds (controls.removeFromLeft (40).reduced (2));
    copyPatternButton.setBounds (controls.removeFromLeft (40).reduced (2));
    controls.removeFromLeft (8);
    viewToggle.setBounds (controls.removeFromLeft (80).reduced (2));
}

void SequencerGrid::mouseDown (const juce::MouseEvent& e)
{
    auto [pad, step] = getStepAtPosition (e.x, e.y);
    if (pad >= 0 && step >= 0)
    {
        auto& pattern = sequencer.getCurrentPattern();
        if (e.mods.isRightButtonDown())
        {
            // Right-click: cycle velocity
            auto& s = pattern.getStep (pad, step);
            if (s.active)
            {
                s.velocity = std::fmod (s.velocity + 0.25f, 1.25f);
                if (s.velocity < 0.25f) s.velocity = 0.25f;
            }
        }
        else
        {
            pattern.toggleStep (pad, step);
        }
        repaint();
    }
}

void SequencerGrid::mouseDrag (const juce::MouseEvent& e)
{
    auto [pad, step] = getStepAtPosition (e.x, e.y);
    if (pad >= 0 && step >= 0)
    {
        auto& pattern = sequencer.getCurrentPattern();
        pattern.setStepActive (pad, step, true);
        repaint();
    }
}

void SequencerGrid::timerCallback()
{
    int step = sequencer.getCurrentStep();
    if (step != lastDrawnStep)
    {
        lastDrawnStep = step;
        repaint();
    }
}

void SequencerGrid::setSelectedPad (int pad)
{
    selectedPad = juce::jlimit (0, SamplerEngine::NUM_PADS - 1, pad);
    repaint();
}

juce::Rectangle<int> SequencerGrid::getStepBounds (int pad, int step) const
{
    auto area = getLocalBounds();
    area.removeFromTop (30); // Controls row
    area.removeFromLeft (22); // Pad labels

    int numSteps = sequencer.getCurrentPattern().getNumSteps();
    int startPad = showAllPads ? 0 : selectedPad;
    int numPads = showAllPads ? SamplerEngine::NUM_PADS : 1;

    int stepW = area.getWidth() / numSteps;
    int padH = area.getHeight() / numPads;

    int row = pad - startPad;
    int x = area.getX() + step * stepW;
    int y = area.getY() + row * padH;

    return { x, y, stepW, padH };
}

std::pair<int, int> SequencerGrid::getStepAtPosition (int x, int y) const
{
    auto area = getLocalBounds();
    area.removeFromTop (30);
    area.removeFromLeft (22);

    if (! area.contains (x, y))
        return { -1, -1 };

    int numSteps = sequencer.getCurrentPattern().getNumSteps();
    int startPad = showAllPads ? 0 : selectedPad;
    int numPads = showAllPads ? SamplerEngine::NUM_PADS : 1;

    int stepW = area.getWidth() / numSteps;
    int padH = area.getHeight() / numPads;

    int step = (x - area.getX()) / stepW;
    int pad = startPad + (y - area.getY()) / padH;

    if (step >= 0 && step < numSteps && pad >= startPad && pad < startPad + numPads)
        return { pad, step };

    return { -1, -1 };
}
