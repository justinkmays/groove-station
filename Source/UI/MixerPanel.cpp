#include "MixerPanel.h"

MixerPanel::MixerPanel (SamplerEngine& eng) : engine (eng)
{
    setupControls();
}

void MixerPanel::setupControls()
{
    auto makeRotary = [this] (juce::Slider& slider, juce::Label& label)
    {
        addAndMakeVisible (slider);
        slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 14);
        slider.onValueChange = [this] { updateEngineFromSliders(); };

        addAndMakeVisible (label);
        label.setFont (juce::Font (9.0f));
        label.setColour (juce::Label::textColourId, Colours_::textSecondary);
        label.setJustificationType (juce::Justification::centred);
    };

    // Volume
    makeRotary (volumeSlider, volumeLabel);
    volumeSlider.setRange (-60.0, 6.0, 0.1);
    volumeSlider.setValue (0.0);
    volumeSlider.setSkewFactorFromMidPoint (-12.0);
    volumeSlider.setDoubleClickReturnValue (true, 0.0);

    // Pan
    makeRotary (panSlider, panLabel);
    panSlider.setRange (-1.0, 1.0, 0.01);
    panSlider.setValue (0.0);
    panSlider.setDoubleClickReturnValue (true, 0.0);

    // Pitch
    makeRotary (pitchSlider, pitchLabel);
    pitchSlider.setRange (-24.0, 24.0, 0.1);
    pitchSlider.setValue (0.0);
    pitchSlider.setDoubleClickReturnValue (true, 0.0);

    // ADSR
    makeRotary (attackSlider, attackLabel);
    attackSlider.setRange (0.001, 2.0, 0.001);
    attackSlider.setValue (0.005);
    attackSlider.setSkewFactorFromMidPoint (0.1);

    makeRotary (decaySlider, decayLabel);
    decaySlider.setRange (0.001, 2.0, 0.001);
    decaySlider.setValue (0.1);
    decaySlider.setSkewFactorFromMidPoint (0.1);

    makeRotary (sustainSlider, sustainLabel);
    sustainSlider.setRange (0.0, 1.0, 0.01);
    sustainSlider.setValue (0.8);

    makeRotary (releaseSlider, releaseLabel);
    releaseSlider.setRange (0.001, 5.0, 0.001);
    releaseSlider.setValue (0.3);
    releaseSlider.setSkewFactorFromMidPoint (0.5);

    // Mute / Solo / Reverse
    for (auto* btn : { &muteButton, &soloButton, &reverseButton })
    {
        addAndMakeVisible (btn);
        btn->setClickingTogglesState (true);
    }

    muteButton.setColour (juce::TextButton::buttonOnColourId, Colours_::muteColour);
    soloButton.setColour (juce::TextButton::buttonOnColourId, Colours_::soloColour);
    reverseButton.setColour (juce::TextButton::buttonOnColourId, Colours_::accentAlt);

    muteButton.onClick = [this]
    {
        engine.getPadState (currentPad).mute = muteButton.getToggleState();
    };
    soloButton.onClick = [this]
    {
        engine.getPadState (currentPad).solo = soloButton.getToggleState();
    };
    reverseButton.onClick = [this]
    {
        engine.getPadState (currentPad).reverse = reverseButton.getToggleState();
        engine.updatePadParameters (currentPad);
    };

    // Master volume
    makeRotary (masterVolumeSlider, masterVolumeLabel);
    masterVolumeSlider.setRange (-60.0, 6.0, 0.1);
    masterVolumeSlider.setValue (0.0);
    masterVolumeSlider.setSkewFactorFromMidPoint (-12.0);
    masterVolumeSlider.setDoubleClickReturnValue (true, 0.0);
    masterVolumeSlider.onValueChange = [this]
    {
        engine.setMasterVolume ((float) masterVolumeSlider.getValue());
    };

    // Load / Clear
    addAndMakeVisible (loadButton);
    addAndMakeVisible (clearButton);

    loadButton.onClick = [this]
    {
        auto chooser = std::make_shared<juce::FileChooser> (
            "Load Sample",
            juce::File::getSpecialLocation (juce::File::userHomeDirectory),
            "*.wav;*.aiff;*.aif;*.mp3;*.flac;*.ogg");

        chooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
            [this, chooser] (const juce::FileChooser& fc)
            {
                auto file = fc.getResult();
                if (file.existsAsFile())
                {
                    engine.loadSample (currentPad, file);
                    if (onParametersChanged) onParametersChanged();
                }
            });
    };

    clearButton.onClick = [this]
    {
        engine.clearSample (currentPad);
        if (onParametersChanged) onParametersChanged();
    };
}

void MixerPanel::paint (juce::Graphics& g)
{
    g.fillAll (Colours_::surface);

    // Section headers
    g.setColour (Colours_::textSecondary);
    g.setFont (juce::Font (11.0f).boldened());

    auto area = getLocalBounds().reduced (8, 0);
    g.drawText ("PAD " + juce::String (currentPad + 1), area.removeFromTop (20), juce::Justification::centredLeft);

    // Divider lines
    g.setColour (Colours_::surfaceLight);
    g.drawHorizontalLine (20, 8.0f, (float) getWidth() - 8.0f);
}

void MixerPanel::resized()
{
    auto area = getLocalBounds().reduced (8, 4);
    area.removeFromTop (20); // Header

    int knobSize = 56;
    int buttonH = 24;

    // Row 1: Vol, Pan, Pitch + buttons
    auto row1 = area.removeFromTop (knobSize + 14);
    auto btnCol = row1.removeFromRight (50);
    muteButton.setBounds (btnCol.removeFromTop (buttonH).reduced (2));
    soloButton.setBounds (btnCol.removeFromTop (buttonH).reduced (2));
    reverseButton.setBounds (btnCol.removeFromTop (buttonH).reduced (2));

    int knobW = row1.getWidth() / 3;
    auto volArea = row1.removeFromLeft (knobW);
    volumeLabel.setBounds (volArea.removeFromTop (14));
    volumeSlider.setBounds (volArea);

    auto panArea = row1.removeFromLeft (knobW);
    panLabel.setBounds (panArea.removeFromTop (14));
    panSlider.setBounds (panArea);

    auto pitchArea = row1;
    pitchLabel.setBounds (pitchArea.removeFromTop (14));
    pitchSlider.setBounds (pitchArea);

    area.removeFromTop (4);

    // Row 2: ADSR
    auto row2 = area.removeFromTop (knobSize + 14);
    int adsrW = row2.getWidth() / 4;

    auto aArea = row2.removeFromLeft (adsrW);
    attackLabel.setBounds (aArea.removeFromTop (14));
    attackSlider.setBounds (aArea);

    auto dArea = row2.removeFromLeft (adsrW);
    decayLabel.setBounds (dArea.removeFromTop (14));
    decaySlider.setBounds (dArea);

    auto sArea = row2.removeFromLeft (adsrW);
    sustainLabel.setBounds (sArea.removeFromTop (14));
    sustainSlider.setBounds (sArea);

    auto rArea = row2;
    releaseLabel.setBounds (rArea.removeFromTop (14));
    releaseSlider.setBounds (rArea);

    area.removeFromTop (4);

    // Row 3: Master + Load/Clear
    auto row3 = area.removeFromTop (knobSize + 14);
    auto masterArea = row3.removeFromLeft (row3.getWidth() / 2);
    masterVolumeLabel.setBounds (masterArea.removeFromTop (14));
    masterVolumeSlider.setBounds (masterArea);

    auto btnArea = row3;
    loadButton.setBounds (btnArea.removeFromTop (buttonH + 4).reduced (4, 2));
    clearButton.setBounds (btnArea.removeFromTop (buttonH + 4).reduced (4, 2));
}

void MixerPanel::updateForPad (int padIndex)
{
    currentPad = juce::jlimit (0, SamplerEngine::NUM_PADS - 1, padIndex);
    updateSlidersFromEngine();
    repaint();
}

void MixerPanel::updateEngineFromSliders()
{
    auto& state = engine.getPadState (currentPad);
    state.volume = (float) volumeSlider.getValue();
    state.pan    = (float) panSlider.getValue();
    state.pitch  = (float) pitchSlider.getValue();

    state.adsr.attack  = (float) attackSlider.getValue();
    state.adsr.decay   = (float) decaySlider.getValue();
    state.adsr.sustain = (float) sustainSlider.getValue();
    state.adsr.release = (float) releaseSlider.getValue();

    engine.updatePadParameters (currentPad);

    if (onParametersChanged)
        onParametersChanged();
}

void MixerPanel::updateSlidersFromEngine()
{
    auto& state = engine.getPadState (currentPad);
    volumeSlider.setValue (state.volume, juce::dontSendNotification);
    panSlider.setValue (state.pan, juce::dontSendNotification);
    pitchSlider.setValue (state.pitch, juce::dontSendNotification);

    attackSlider.setValue  (state.adsr.attack,  juce::dontSendNotification);
    decaySlider.setValue   (state.adsr.decay,   juce::dontSendNotification);
    sustainSlider.setValue (state.adsr.sustain, juce::dontSendNotification);
    releaseSlider.setValue (state.adsr.release, juce::dontSendNotification);

    muteButton.setToggleState (state.mute, juce::dontSendNotification);
    soloButton.setToggleState (state.solo, juce::dontSendNotification);
    reverseButton.setToggleState (state.reverse, juce::dontSendNotification);

    masterVolumeSlider.setValue (engine.getMasterVolume(), juce::dontSendNotification);
}
