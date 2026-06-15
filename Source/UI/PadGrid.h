#pragma once
#include <JuceHeader.h>
#include "../Audio/SamplerEngine.h"
#include "CustomLookAndFeel.h"

class PadGrid : public juce::Component,
                public juce::FileDragAndDropTarget
{
public:
    PadGrid (SamplerEngine& engine);

    void paint (juce::Graphics& g) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent& e) override;
    void mouseUp (const juce::MouseEvent& e) override;

    // Drag and drop
    bool isInterestedInFileDrag (const juce::StringArray& files) override;
    void filesDropped (const juce::StringArray& files, int x, int y) override;
    void fileDragEnter (const juce::StringArray&, int, int) override;
    void fileDragExit (const juce::StringArray&) override;

    int getSelectedPad() const { return selectedPad; }
    void setSelectedPad (int pad);

    std::function<void (int padIndex)> onPadSelected;
    std::function<void (int padIndex)> onSampleLoaded;

private:
    SamplerEngine& engine;
    int selectedPad = 0;
    int hoveredPad = -1;
    bool dragOver = false;

    int getPadAtPosition (int x, int y) const;
    juce::Rectangle<int> getPadBounds (int padIndex) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PadGrid)
};
