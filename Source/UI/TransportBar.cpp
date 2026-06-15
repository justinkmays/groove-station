#include "TransportBar.h"

TransportBar::TransportBar (StepSequencer& seq) : sequencer (seq)
{
    setupControls();
    startTimerHz (30);
}

TransportBar::~TransportBar()
{
    stopTimer();
}

void TransportBar::setupControls()
{
    // Play button
    addAndMakeVisible (playButton);
    playButton.setClickingTogglesState (true);
    playButton.onClick = [this]
    {
        if (playButton.getToggleState())
            sequencer.play();
        else
            sequencer.pause();
    };

    // Stop button
    addAndMakeVisible (stopButton);
    stopButton.onClick = [this]
    {
        sequencer.stop();
        playButton.setToggleState (false, juce::dontSendNotification);
    };

    // Record button
    addAndMakeVisible (recordButton);
    recordButton.setClickingTogglesState (true);
    recordButton.setColour (juce::TextButton::buttonOnColourId, Colours_::meterRed);

    // BPM
    addAndMakeVisible (bpmSlider);
    bpmSlider.setRange (30.0, 300.0, 0.1);
    bpmSlider.setValue (sequencer.getBPM());
    bpmSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    bpmSlider.setTextBoxStyle (juce::Slider::TextBoxRight, false, 50, 20);
    bpmSlider.onValueChange = [this] { sequencer.setBPM (bpmSlider.getValue()); };
    addAndMakeVisible (bpmLabel);

    // Swing
    addAndMakeVisible (swingSlider);
    swingSlider.setRange (0.0, 1.0, 0.01);
    swingSlider.setValue (sequencer.getSwing());
    swingSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    swingSlider.setTextBoxStyle (juce::Slider::TextBoxRight, false, 40, 20);
    swingSlider.onValueChange = [this] { sequencer.setSwing ((float) swingSlider.getValue()); };
    addAndMakeVisible (swingLabel);

    // Quantize
    addAndMakeVisible (quantizeBox);
    quantizeBox.addItem ("1/4",  1);
    quantizeBox.addItem ("1/8",  2);
    quantizeBox.addItem ("1/16", 3);
    quantizeBox.addItem ("1/32", 4);
    quantizeBox.setSelectedId (3);
    quantizeBox.onChange = [this]
    {
        switch (quantizeBox.getSelectedId())
        {
            case 1: sequencer.setQuantize (StepSequencer::Quantize::Q_1_4);  break;
            case 2: sequencer.setQuantize (StepSequencer::Quantize::Q_1_8);  break;
            case 3: sequencer.setQuantize (StepSequencer::Quantize::Q_1_16); break;
            case 4: sequencer.setQuantize (StepSequencer::Quantize::Q_1_32); break;
        }
    };
    addAndMakeVisible (quantizeLabel);

    // Time signature
    addAndMakeVisible (timeSigBox);
    timeSigBox.addItem ("4/4", 1);
    timeSigBox.addItem ("3/4", 2);
    timeSigBox.addItem ("6/8", 3);
    timeSigBox.addItem ("7/8", 4);
    timeSigBox.setSelectedId (1);
    timeSigBox.onChange = [this]
    {
        switch (timeSigBox.getSelectedId())
        {
            case 1: sequencer.setTimeSignature (4, 4); break;
            case 2: sequencer.setTimeSignature (3, 4); break;
            case 3: sequencer.setTimeSignature (6, 8); break;
            case 4: sequencer.setTimeSignature (7, 8); break;
        }
    };
    addAndMakeVisible (timeSigLabel);

    // Position display
    addAndMakeVisible (positionLabel);
    positionLabel.setFont (juce::Font (16.0f).boldened());
    positionLabel.setJustificationType (juce::Justification::centred);
    positionLabel.setColour (juce::Label::textColourId, Colours_::stepCurrent);

    // Colours
    for (auto* label : { &bpmLabel, &swingLabel, &quantizeLabel, &timeSigLabel })
    {
        label->setFont (juce::Font (10.0f));
        label->setColour (juce::Label::textColourId, Colours_::textSecondary);
        label->setJustificationType (juce::Justification::centred);
    }
}

void TransportBar::paint (juce::Graphics& g)
{
    g.fillAll (Colours_::surface);
    g.setColour (Colours_::surfaceLight);
    g.drawLine (0.0f, (float) getHeight(), (float) getWidth(), (float) getHeight(), 1.0f);
}

void TransportBar::resized()
{
    auto area = getLocalBounds().reduced (8, 4);

    playButton.setBounds   (area.removeFromLeft (60).reduced (2));
    stopButton.setBounds   (area.removeFromLeft (60).reduced (2));
    recordButton.setBounds (area.removeFromLeft (60).reduced (2));

    area.removeFromLeft (16);

    // BPM
    auto bpmArea = area.removeFromLeft (160);
    bpmLabel.setBounds (bpmArea.removeFromTop (14));
    bpmSlider.setBounds (bpmArea);

    area.removeFromLeft (8);

    // Swing
    auto swingArea = area.removeFromLeft (120);
    swingLabel.setBounds (swingArea.removeFromTop (14));
    swingSlider.setBounds (swingArea);

    area.removeFromLeft (8);

    // Quantize
    auto quantArea = area.removeFromLeft (80);
    quantizeLabel.setBounds (quantArea.removeFromTop (14));
    quantizeBox.setBounds (quantArea.reduced (0, 2));

    area.removeFromLeft (8);

    // Time signature
    auto timeSigArea = area.removeFromLeft (80);
    timeSigLabel.setBounds (timeSigArea.removeFromTop (14));
    timeSigBox.setBounds (timeSigArea.reduced (0, 2));

    area.removeFromLeft (16);

    // Position display
    positionLabel.setBounds (area.removeFromLeft (120));
}

void TransportBar::timerCallback()
{
    int step = sequencer.getCurrentStep();
    int bar = sequencer.getCurrentBar();
    int beat = step / 4;
    int tick = step % 4;

    positionLabel.setText (juce::String (bar + 1) + "." + juce::String (beat + 1) + "." + juce::String (tick + 1),
                           juce::dontSendNotification);

    playButton.setToggleState (sequencer.isPlaying(), juce::dontSendNotification);
}
