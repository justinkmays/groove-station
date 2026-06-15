#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "UI/PadGrid.h"
#include "UI/WaveformDisplay.h"
#include "UI/TransportBar.h"
#include "UI/MixerPanel.h"
#include "UI/EffectsPanel.h"
#include "UI/SequencerGrid.h"
#include "UI/CustomLookAndFeel.h"

class GrooveStationEditor : public juce::AudioProcessorEditor
{
public:
    explicit GrooveStationEditor (GrooveStationProcessor&);
    ~GrooveStationEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    GrooveStationProcessor& processor;
    CustomLookAndFeel customLookAndFeel;

    // Header
    juce::Label titleLabel;

    // Main sections
    TransportBar    transportBar;
    PadGrid         padGrid;
    WaveformDisplay waveformDisplay;
    MixerPanel      mixerPanel;
    EffectsPanel    effectsPanel;
    SequencerGrid   sequencerGrid;

    // Keyboard for MIDI
    juce::MidiKeyboardState keyboardState;
    juce::MidiKeyboardComponent keyboard;

    void onPadSelected (int padIndex);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GrooveStationEditor)
};
