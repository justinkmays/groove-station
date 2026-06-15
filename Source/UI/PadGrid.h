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
    void setSelectedPad (int absolutePadIndex);

    // Callbacks pass absolute pad indices
    std::function<void (int absolutePadIndex)> onPadSelected;
    std::function<void (int absolutePadIndex)> onSampleLoaded;
    std::function<void (int bank)> onBankChanged;

private:
    SamplerEngine& engine;
    int selectedPad = 0; // absolute pad index
    int hoveredPad = -1;
    bool dragOver = false;

    juce::TextButton bankButtons[SamplerEngine::NUM_BANKS];

    int getPadAtPosition (int x, int y) const; // returns bank-relative index
    juce::Rectangle<int> getPadBounds (int bankRelativeIndex) const;
    juce::Rectangle<int> getGridArea() const;
    void updateBankButtonColours();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PadGrid)
};
