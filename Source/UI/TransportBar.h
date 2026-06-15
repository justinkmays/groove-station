#pragma once
#include <JuceHeader.h>
#include "../Sequencer/StepSequencer.h"
#include "CustomLookAndFeel.h"

class TransportBar : public juce::Component,
                     public juce::Timer
{
public:
    TransportBar (StepSequencer& sequencer);
    ~TransportBar() override;

    void paint (juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;

private:
    StepSequencer& sequencer;

    juce::TextButton playButton   { "PLAY" };
    juce::TextButton stopButton   { "STOP" };
    juce::TextButton recordButton { "REC" };

    juce::Slider bpmSlider;
    juce::Label  bpmLabel { {}, "BPM" };

    juce::Slider swingSlider;
    juce::Label  swingLabel { {}, "SWING" };

    juce::ComboBox quantizeBox;
    juce::Label    quantizeLabel { {}, "QUANTIZE" };

    juce::ComboBox timeSigBox;
    juce::Label    timeSigLabel { {}, "TIME SIG" };

    juce::Label positionLabel;

    void setupControls();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TransportBar)
};
