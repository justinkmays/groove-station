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
      soundBrowser (p.getSamplerEngine()),
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
    addAndMakeVisible (soundBrowser);
    addAndMakeVisible (keyboard);

    // Keyboard styling
    keyboard.setColour (juce::MidiKeyboardComponent::keyDownOverlayColourId, Colours_::accent);
    keyboard.setColour (juce::MidiKeyboardComponent::whiteNoteColourId, Colours_::surface);
    keyboard.setColour (juce::MidiKeyboardComponent::blackNoteColourId, Colours_::background);
    keyboard.setColour (juce::MidiKeyboardComponent::keySeparatorLineColourId, Colours_::surfaceLight);
    keyboard.setOctaveForMiddleC (3);

    // Pad selected callback: update all panels with absolute pad index
    padGrid.onPadSelected = [this] (int absPad) { onPadSelected (absPad); };
    padGrid.onSampleLoaded = [this] (int absPad)
    {
        onPadSelected (absPad);
        waveformDisplay.repaint();
    };

    // Bank changed: update sequencer grid
    padGrid.onBankChanged = [this] (int /*bank*/)
    {
        sequencerGrid.repaint();
    };

    mixerPanel.onParametersChanged = [this]
    {
        waveformDisplay.repaint();
        padGrid.repaint();
    };

    // Sound browser: loaded file triggers pad update
    soundBrowser.onSampleLoaded = [this] (int absPad)
    {
        onPadSelected (absPad);
        padGrid.repaint();
        waveformDisplay.repaint();
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

    // Version info
    g.setColour (Colours_::textSecondary.withAlpha (0.4f));
    g.setFont (9.0f);
    g.drawText ("v1.1.0", getLocalBounds().reduced (8), juce::Justification::bottomRight);
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
    sequencerGrid.setBounds (area.removeFromBottom (area.getHeight() * 30 / 100));

    // Remaining area: left side = sound browser + pads + waveform, right = mixer + effects
    auto mainArea = area;

    // Right side: mixer and effects panels
    auto rightPanel = mainArea.removeFromRight (240);
    mixerPanel.setBounds (rightPanel.removeFromTop (rightPanel.getHeight() / 2));
    effectsPanel.setBounds (rightPanel);

    // Left side split: sound browser on far left, pad grid + waveform in center
    auto soundBrowserArea = mainArea.removeFromLeft (juce::jmin (220, mainArea.getWidth() / 4));
    soundBrowser.setBounds (soundBrowserArea);

    // Center: waveform on top, pad grid below
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
    soundBrowser.setTargetPad (padIndex);
}
