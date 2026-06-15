#pragma once
#include <JuceHeader.h>
#include "../Audio/SamplerEngine.h"
#include "CustomLookAndFeel.h"

class WaveformDisplay : public juce::Component
{
public:
    WaveformDisplay (SamplerEngine& engine);

    void paint (juce::Graphics& g) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent& e) override;
    void mouseDrag (const juce::MouseEvent& e) override;

    void setPadIndex (int index);
    int getPadIndex() const { return currentPad; }

    std::function<void (float start, float end)> onRegionChanged;

private:
    SamplerEngine& engine;
    int currentPad = 0;

    // Dragging state for start/end markers
    enum class DragTarget { None, Start, End };
    DragTarget dragTarget = DragTarget::None;

    float positionToNormalized (int x) const;
    int normalizedToPosition (float n) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WaveformDisplay)
};
