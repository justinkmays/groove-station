#pragma once
#include <JuceHeader.h>
#include "../Audio/SamplerEngine.h"
#include "CustomLookAndFeel.h"

class MixerPanel : public juce::Component
{
public:
    MixerPanel (SamplerEngine& engine);

    void paint (juce::Graphics& g) override;
    void resized() override;
    void updateForPad (int padIndex);

    std::function<void()> onParametersChanged;

private:
    SamplerEngine& engine;
    int currentPad = 0;

    // Volume & Pan
    juce::Slider volumeSlider;
    juce::Label  volumeLabel { {}, "VOL" };
    juce::Slider panSlider;
    juce::Label  panLabel { {}, "PAN" };

    // Pitch
    juce::Slider pitchSlider;
    juce::Label  pitchLabel { {}, "PITCH" };

    // ADSR
    juce::Slider attackSlider, decaySlider, sustainSlider, releaseSlider;
    juce::Label  attackLabel { {}, "A" }, decayLabel { {}, "D" },
                 sustainLabel { {}, "S" }, releaseLabel { {}, "R" };

    // Mute/Solo
    juce::TextButton muteButton { "M" };
    juce::TextButton soloButton { "S" };
    juce::TextButton reverseButton { "REV" };

    // Master
    juce::Slider masterVolumeSlider;
    juce::Label  masterVolumeLabel { {}, "MASTER" };

    // Load/Clear
    juce::TextButton loadButton  { "LOAD" };
    juce::TextButton clearButton { "CLEAR" };

    void setupControls();
    void updateEngineFromSliders();
    void updateSlidersFromEngine();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MixerPanel)
};
