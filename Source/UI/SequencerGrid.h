#pragma once
#include <JuceHeader.h>
#include "../Sequencer/StepSequencer.h"
#include "../Audio/SamplerEngine.h"
#include "CustomLookAndFeel.h"

class SequencerGrid : public juce::Component,
                      public juce::Timer
{
public:
    SequencerGrid (StepSequencer& sequencer, SamplerEngine& engine);
    ~SequencerGrid() override;

    void paint (juce::Graphics& g) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent& e) override;
    void mouseDrag (const juce::MouseEvent& e) override;
    void timerCallback() override;

    void setSelectedPad (int pad);
    int getSelectedPad() const { return selectedPad; }

    // Pattern controls
    juce::ComboBox patternSelector;
    juce::Slider stepsSlider;
    juce::TextButton clearPatternButton { "CLR" };
    juce::TextButton copyPatternButton { "CPY" };

private:
    StepSequencer& sequencer;
    SamplerEngine& engine;
    int selectedPad = 0;
    int lastDrawnStep = -1;

    // Show all pads or just selected
    bool showAllPads = true;
    juce::TextButton viewToggle { "ALL PADS" };

    juce::Rectangle<int> getStepBounds (int pad, int step) const;
    std::pair<int, int> getStepAtPosition (int x, int y) const;

    void setupControls();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SequencerGrid)
};
