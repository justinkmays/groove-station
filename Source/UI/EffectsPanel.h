#pragma once
#include <JuceHeader.h>
#include "../Audio/SamplerEngine.h"
#include "CustomLookAndFeel.h"

class EffectsPanel : public juce::Component
{
public:
    EffectsPanel (SamplerEngine& engine);

    void paint (juce::Graphics& g) override;
    void resized() override;
    void updateForPad (int padIndex);

private:
    SamplerEngine& engine;
    int currentPad = 0;

    // Tab: per-pad effects vs master effects
    juce::TextButton padEffectsTab { "PAD FX" };
    juce::TextButton masterEffectsTab { "MASTER FX" };
    bool showMasterEffects = false;

    // Filter
    juce::Slider filterCutoffSlider, filterResoSlider;
    juce::ComboBox filterTypeBox;
    juce::Label filterCutoffLabel { {}, "CUTOFF" }, filterResoLabel { {}, "RESO" },
                filterTypeLabel { {}, "FILTER" };

    // Reverb
    juce::Slider reverbMixSlider, reverbRoomSlider, reverbDampSlider;
    juce::Label  reverbMixLabel { {}, "MIX" }, reverbRoomLabel { {}, "ROOM" },
                 reverbDampLabel { {}, "DAMP" };

    // Delay
    juce::Slider delayMixSlider, delayTimeSlider, delayFeedbackSlider;
    juce::Label  delayMixLabel { {}, "MIX" }, delayTimeLabel { {}, "TIME" },
                 delayFeedbackLabel { {}, "FB" };

    // Distortion
    juce::Slider distAmountSlider, distMixSlider;
    juce::Label  distAmountLabel { {}, "DRIVE" }, distMixLabel { {}, "MIX" };

    // EQ
    juce::Slider eqLowSlider, eqMidSlider, eqHighSlider;
    juce::Label  eqLowLabel { {}, "LOW" }, eqMidLabel { {}, "MID" },
                 eqHighLabel { {}, "HIGH" };

    void setupControls();
    void updateEngineFromSliders();
    void updateSlidersFromEngine();
    EffectsChain::Parameters getActiveParams();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EffectsPanel)
};
