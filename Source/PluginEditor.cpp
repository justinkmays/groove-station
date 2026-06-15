#include "PluginEditor.h"

GrooveStationEditor::GrooveStationEditor (GrooveStationProcessor& p)
    : AudioProcessorEditor (&p),
      processor (p),
      transportBar (p.getStepSequencer()),
      padGrid (p.getSamplerEngine()),
      waveformDisplay (p.getSamplerEngine()),
      mixerPanel (p.getSamplerEngine()),
      effectsPanel (p.getSamplerEngine()),
      sequencerGrid (p.getStepSequencer(), p.getSamplerEngine()),
      keyboard (keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    setLookAndFeel (&customLookAndFeel);
    setSize (1280, 800);
    setResizable (true, true);
    setResizeLimits (900, 600, 2560, 1600);

    // Title
    addAndMakeVisible (titleLabel);
    titleLabel.setText ("GROOVE STATION", juce::dontSendNotification);
    titleLabel.setFont (juce::Font (20.0f).boldened());
    titleLabel.setColour (juce::Label::textColourId, Colours_::accent);
    titleLabel.setJustificationType (juce::Justification::centredLeft);

    // Main components
    addAndMakeVisible (transportBar);
    addAndMakeVisible (padGrid);
    addAndMakeVisible (waveformDisplay);
    addAndMakeVisible (mixerPanel);
    addAndMakeVisible (effectsPanel);
    addAndMakeVisible (sequencerGrid);
    addAndMakeVisible (keyboard);

    // Keyboard styling
    keyboard.setColour (juce::MidiKeyboardComponent::keyDownOverlayColourId, Colours_::accent);
    keyboard.setColour (juce::MidiKeyboardComponent::whiteNoteColourId, Colours_::surface);
    keyboard.setColour (juce::MidiKeyboardComponent::blackNoteColourId, Colours_::background);
    keyboard.setColour (juce::MidiKeyboardComponent::keySeparatorLineColourId, Colours_::surfaceLight);
    keyboard.setOctaveForMiddleC (3);

    // Callbacks
    padGrid.onPadSelected = [this] (int pad) { onPadSelected (pad); };
    padGrid.onSampleLoaded = [this] (int pad)
    {
        onPadSelected (pad);
        waveformDisplay.repaint();
    };

    mixerPanel.onParametersChanged = [this]
    {
        waveformDisplay.repaint();
        padGrid.repaint();
    };

    // Initial state
    onPadSelected (0);
}

GrooveStationEditor::~GrooveStationEditor()
{
    setLookAndFeel (nullptr);
}

void GrooveStationEditor::paint (juce::Graphics& g)
{
    g.fillAll (Colours_::background);

    // Subtle grid lines / section borders
    g.setColour (Colours_::surfaceLight.withAlpha (0.3f));

    // Version info
    g.setColour (Colours_::textSecondary.withAlpha (0.4f));
    g.setFont (9.0f);
    g.drawText ("v1.0.0", getLocalBounds().reduced (8), juce::Justification::bottomRight);
}

void GrooveStationEditor::resized()
{
    auto area = getLocalBounds();

    // Title bar
    auto headerArea = area.removeFromTop (32);
    titleLabel.setBounds (headerArea.reduced (10, 0));

    // Transport bar
    transportBar.setBounds (area.removeFromTop (50));

    // Keyboard at bottom
    keyboard.setBounds (area.removeFromBottom (50));

    // Sequencer grid in lower portion
    sequencerGrid.setBounds (area.removeFromBottom (area.getHeight() * 35 / 100));

    // Remaining area split: left = pads + waveform, right = mixer + effects
    auto mainArea = area;

    // Right side: mixer and effects panels
    auto rightPanel = mainArea.removeFromRight (240);
    mixerPanel.setBounds (rightPanel.removeFromTop (rightPanel.getHeight() / 2));
    effectsPanel.setBounds (rightPanel);

    // Left side: pad grid and waveform
    auto leftPanel = mainArea;
    waveformDisplay.setBounds (leftPanel.removeFromTop (leftPanel.getHeight() * 30 / 100));
    padGrid.setBounds (leftPanel);
}

void GrooveStationEditor::onPadSelected (int padIndex)
{
    waveformDisplay.setPadIndex (padIndex);
    mixerPanel.updateForPad (padIndex);
    effectsPanel.updateForPad (padIndex);
    sequencerGrid.setSelectedPad (padIndex);
}
