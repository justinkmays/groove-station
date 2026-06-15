#include "EffectsPanel.h"

EffectsPanel::EffectsPanel (SamplerEngine& eng) : engine (eng)
{
    setupControls();
}

void EffectsPanel::setupControls()
{
    auto makeRotary = [this] (juce::Slider& slider, juce::Label& label)
    {
        addAndMakeVisible (slider);
        slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 48, 12);
        slider.onValueChange = [this] { updateEngineFromSliders(); };

        addAndMakeVisible (label);
        label.setFont (juce::Font (8.0f));
        label.setColour (juce::Label::textColourId, Colours_::textSecondary);
        label.setJustificationType (juce::Justification::centred);
    };

    // Tabs
    addAndMakeVisible (padEffectsTab);
    addAndMakeVisible (masterEffectsTab);
    padEffectsTab.setClickingTogglesState (true);
    masterEffectsTab.setClickingTogglesState (true);
    padEffectsTab.setRadioGroupId (100);
    masterEffectsTab.setRadioGroupId (100);
    padEffectsTab.setToggleState (true, juce::dontSendNotification);

    padEffectsTab.onClick = [this] { showMasterEffects = false; updateSlidersFromEngine(); repaint(); };
    masterEffectsTab.onClick = [this] { showMasterEffects = true; updateSlidersFromEngine(); repaint(); };

    // Filter
    makeRotary (filterCutoffSlider, filterCutoffLabel);
    filterCutoffSlider.setRange (20.0, 20000.0, 1.0);
    filterCutoffSlider.setValue (20000.0);
    filterCutoffSlider.setSkewFactorFromMidPoint (1000.0);

    makeRotary (filterResoSlider, filterResoLabel);
    filterResoSlider.setRange (0.1, 10.0, 0.01);
    filterResoSlider.setValue (0.707);
    filterResoSlider.setSkewFactorFromMidPoint (1.0);

    addAndMakeVisible (filterTypeBox);
    filterTypeBox.addItem ("LPF", 1);
    filterTypeBox.addItem ("HPF", 2);
    filterTypeBox.addItem ("BPF", 3);
    filterTypeBox.setSelectedId (1);
    filterTypeBox.onChange = [this] { updateEngineFromSliders(); };
    addAndMakeVisible (filterTypeLabel);
    filterTypeLabel.setFont (juce::Font (8.0f));
    filterTypeLabel.setColour (juce::Label::textColourId, Colours_::textSecondary);
    filterTypeLabel.setJustificationType (juce::Justification::centred);

    // Reverb
    makeRotary (reverbMixSlider, reverbMixLabel);
    reverbMixSlider.setRange (0.0, 1.0, 0.01);
    makeRotary (reverbRoomSlider, reverbRoomLabel);
    reverbRoomSlider.setRange (0.0, 1.0, 0.01);
    reverbRoomSlider.setValue (0.5);
    makeRotary (reverbDampSlider, reverbDampLabel);
    reverbDampSlider.setRange (0.0, 1.0, 0.01);
    reverbDampSlider.setValue (0.5);

    // Delay
    makeRotary (delayMixSlider, delayMixLabel);
    delayMixSlider.setRange (0.0, 1.0, 0.01);
    makeRotary (delayTimeSlider, delayTimeLabel);
    delayTimeSlider.setRange (0.01, 2.0, 0.01);
    delayTimeSlider.setValue (0.3);
    delayTimeSlider.setSkewFactorFromMidPoint (0.3);
    makeRotary (delayFeedbackSlider, delayFeedbackLabel);
    delayFeedbackSlider.setRange (0.0, 0.95, 0.01);
    delayFeedbackSlider.setValue (0.4);

    // Distortion
    makeRotary (distAmountSlider, distAmountLabel);
    distAmountSlider.setRange (0.0, 1.0, 0.01);
    makeRotary (distMixSlider, distMixLabel);
    distMixSlider.setRange (0.0, 1.0, 0.01);

    // EQ
    makeRotary (eqLowSlider, eqLowLabel);
    eqLowSlider.setRange (-12.0, 12.0, 0.1);
    eqLowSlider.setDoubleClickReturnValue (true, 0.0);
    makeRotary (eqMidSlider, eqMidLabel);
    eqMidSlider.setRange (-12.0, 12.0, 0.1);
    eqMidSlider.setDoubleClickReturnValue (true, 0.0);
    makeRotary (eqHighSlider, eqHighLabel);
    eqHighSlider.setRange (-12.0, 12.0, 0.1);
    eqHighSlider.setDoubleClickReturnValue (true, 0.0);
}

void EffectsPanel::paint (juce::Graphics& g)
{
    g.fillAll (Colours_::surface);

    // Section headers
    g.setColour (Colours_::textSecondary);
    g.setFont (juce::Font (10.0f).boldened());

    int y = 34;
    int sectionH = (getHeight() - 34) / 5;

    g.drawText ("FILTER", 8, y, 60, 14, juce::Justification::centredLeft);
    y += sectionH;
    g.drawText ("REVERB", 8, y, 60, 14, juce::Justification::centredLeft);
    y += sectionH;
    g.drawText ("DELAY", 8, y, 60, 14, juce::Justification::centredLeft);
    y += sectionH;
    g.drawText ("DRIVE", 8, y, 60, 14, juce::Justification::centredLeft);
    y += sectionH;
    g.drawText ("EQ", 8, y, 60, 14, juce::Justification::centredLeft);
}

void EffectsPanel::resized()
{
    auto area = getLocalBounds().reduced (4, 2);

    // Tab buttons
    auto tabRow = area.removeFromTop (28);
    padEffectsTab.setBounds (tabRow.removeFromLeft (tabRow.getWidth() / 2).reduced (2));
    masterEffectsTab.setBounds (tabRow.reduced (2));

    area.removeFromTop (2);

    int sectionH = area.getHeight() / 5;
    int knobSize = sectionH - 16;
    int labelH = 12;

    auto layoutRow = [&] (juce::Slider* sliders[], juce::Label* labels[], int count,
                          juce::Rectangle<int> rowArea)
    {
        rowArea.removeFromTop (14); // Section label
        int w = rowArea.getWidth() / count;
        for (int i = 0; i < count; ++i)
        {
            auto col = rowArea.removeFromLeft (w);
            labels[i]->setBounds (col.removeFromTop (labelH));
            sliders[i]->setBounds (col);
        }
    };

    // Filter
    {
        auto row = area.removeFromTop (sectionH);
        row.removeFromTop (14);
        int w = row.getWidth() / 3;
        auto col1 = row.removeFromLeft (w);
        filterTypeLabel.setBounds (col1.removeFromTop (labelH));
        filterTypeBox.setBounds (col1.reduced (2, (col1.getHeight() - 20) / 2));

        juce::Slider* s[] = { &filterCutoffSlider, &filterResoSlider };
        juce::Label* l[] = { &filterCutoffLabel, &filterResoLabel };
        int sw = row.getWidth() / 2;
        for (int i = 0; i < 2; ++i)
        {
            auto col = row.removeFromLeft (sw);
            l[i]->setBounds (col.removeFromTop (labelH));
            s[i]->setBounds (col);
        }
    }

    // Reverb
    {
        auto row = area.removeFromTop (sectionH);
        juce::Slider* s[] = { &reverbMixSlider, &reverbRoomSlider, &reverbDampSlider };
        juce::Label* l[] = { &reverbMixLabel, &reverbRoomLabel, &reverbDampLabel };
        layoutRow (s, l, 3, row);
    }

    // Delay
    {
        auto row = area.removeFromTop (sectionH);
        juce::Slider* s[] = { &delayMixSlider, &delayTimeSlider, &delayFeedbackSlider };
        juce::Label* l[] = { &delayMixLabel, &delayTimeLabel, &delayFeedbackLabel };
        layoutRow (s, l, 3, row);
    }

    // Distortion
    {
        auto row = area.removeFromTop (sectionH);
        juce::Slider* s[] = { &distAmountSlider, &distMixSlider };
        juce::Label* l[] = { &distAmountLabel, &distMixLabel };
        layoutRow (s, l, 2, row);
    }

    // EQ
    {
        auto row = area;
        juce::Slider* s[] = { &eqLowSlider, &eqMidSlider, &eqHighSlider };
        juce::Label* l[] = { &eqLowLabel, &eqMidLabel, &eqHighLabel };
        layoutRow (s, l, 3, row);
    }
}

void EffectsPanel::updateForPad (int padIndex)
{
    currentPad = juce::jlimit (0, SamplerEngine::TOTAL_PADS - 1, padIndex);
    updateSlidersFromEngine();
}

EffectsChain::Parameters EffectsPanel::getActiveParams()
{
    if (showMasterEffects)
        return engine.getMasterEffects().getParameters();
    return engine.getPadState (currentPad).effects;
}

void EffectsPanel::updateEngineFromSliders()
{
    auto& params = engine.getPadState (currentPad).effects;
    if (showMasterEffects)
    {
        EffectsChain::Parameters p;
        p.filterCutoff    = (float) filterCutoffSlider.getValue();
        p.filterResonance = (float) filterResoSlider.getValue();
        p.filterType      = filterTypeBox.getSelectedId() - 1;
        p.reverbMix       = (float) reverbMixSlider.getValue();
        p.reverbRoom      = (float) reverbRoomSlider.getValue();
        p.reverbDamp      = (float) reverbDampSlider.getValue();
        p.delayMix        = (float) delayMixSlider.getValue();
        p.delayTime       = (float) delayTimeSlider.getValue();
        p.delayFeedback   = (float) delayFeedbackSlider.getValue();
        p.distortionAmount = (float) distAmountSlider.getValue();
        p.distortionMix   = (float) distMixSlider.getValue();
        p.eqLowGain       = (float) eqLowSlider.getValue();
        p.eqMidGain       = (float) eqMidSlider.getValue();
        p.eqHighGain      = (float) eqHighSlider.getValue();
        engine.getMasterEffects().setParameters (p);
    }
    else
    {
        params.filterCutoff    = (float) filterCutoffSlider.getValue();
        params.filterResonance = (float) filterResoSlider.getValue();
        params.filterType      = filterTypeBox.getSelectedId() - 1;
        params.reverbMix       = (float) reverbMixSlider.getValue();
        params.reverbRoom      = (float) reverbRoomSlider.getValue();
        params.reverbDamp      = (float) reverbDampSlider.getValue();
        params.delayMix        = (float) delayMixSlider.getValue();
        params.delayTime       = (float) delayTimeSlider.getValue();
        params.delayFeedback   = (float) delayFeedbackSlider.getValue();
        params.distortionAmount = (float) distAmountSlider.getValue();
        params.distortionMix   = (float) distMixSlider.getValue();
        params.eqLowGain       = (float) eqLowSlider.getValue();
        params.eqMidGain       = (float) eqMidSlider.getValue();
        params.eqHighGain      = (float) eqHighSlider.getValue();
        engine.updatePadParameters (currentPad);
    }
}

void EffectsPanel::updateSlidersFromEngine()
{
    EffectsChain::Parameters params;
    if (showMasterEffects)
        params = engine.getMasterEffects().getParameters();
    else
        params = engine.getPadState (currentPad).effects;

    filterCutoffSlider.setValue (params.filterCutoff, juce::dontSendNotification);
    filterResoSlider.setValue (params.filterResonance, juce::dontSendNotification);
    filterTypeBox.setSelectedId (params.filterType + 1, juce::dontSendNotification);
    reverbMixSlider.setValue (params.reverbMix, juce::dontSendNotification);
    reverbRoomSlider.setValue (params.reverbRoom, juce::dontSendNotification);
    reverbDampSlider.setValue (params.reverbDamp, juce::dontSendNotification);
    delayMixSlider.setValue (params.delayMix, juce::dontSendNotification);
    delayTimeSlider.setValue (params.delayTime, juce::dontSendNotification);
    delayFeedbackSlider.setValue (params.delayFeedback, juce::dontSendNotification);
    distAmountSlider.setValue (params.distortionAmount, juce::dontSendNotification);
    distMixSlider.setValue (params.distortionMix, juce::dontSendNotification);
    eqLowSlider.setValue (params.eqLowGain, juce::dontSendNotification);
    eqMidSlider.setValue (params.eqMidGain, juce::dontSendNotification);
    eqHighSlider.setValue (params.eqHighGain, juce::dontSendNotification);
}
