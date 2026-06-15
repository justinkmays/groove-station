#pragma once
#include <JuceHeader.h>

class Pattern
{
public:
    static constexpr int MAX_STEPS = 64;
    static constexpr int NUM_PADS  = 16;

    struct Step
    {
        bool  active   = false;
        float velocity = 1.0f;
        float probability = 1.0f; // 0..1, chance of firing
    };

    Pattern();
    Pattern (int numSteps);

    void setNumSteps (int steps);
    int  getNumSteps() const { return numSteps; }

    Step& getStep (int pad, int step);
    const Step& getStep (int pad, int step) const;

    void toggleStep (int pad, int step);
    void setStepActive (int pad, int step, bool active);
    void setStepVelocity (int pad, int step, float vel);
    void setStepProbability (int pad, int step, float prob);

    void clear();
    void clearPad (int pad);
    Pattern copy() const;

    juce::String getName() const { return name; }
    void setName (const juce::String& n) { name = n; }

private:
    int numSteps = 16;
    Step steps[NUM_PADS][MAX_STEPS];
    juce::String name = "Pattern 1";
};
