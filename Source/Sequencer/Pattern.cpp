#include "Pattern.h"

Pattern::Pattern()
{
    clear();
}

Pattern::Pattern (int steps) : numSteps (juce::jlimit (1, MAX_STEPS, steps))
{
    clear();
}

void Pattern::setNumSteps (int steps)
{
    numSteps = juce::jlimit (1, MAX_STEPS, steps);
}

Pattern::Step& Pattern::getStep (int pad, int step)
{
    jassert (pad >= 0 && pad < NUM_PADS);
    jassert (step >= 0 && step < MAX_STEPS);
    return steps[pad][step];
}

const Pattern::Step& Pattern::getStep (int pad, int step) const
{
    jassert (pad >= 0 && pad < NUM_PADS);
    jassert (step >= 0 && step < MAX_STEPS);
    return steps[pad][step];
}

void Pattern::toggleStep (int pad, int step)
{
    if (pad >= 0 && pad < NUM_PADS && step >= 0 && step < MAX_STEPS)
        steps[pad][step].active = ! steps[pad][step].active;
}

void Pattern::setStepActive (int pad, int step, bool active)
{
    if (pad >= 0 && pad < NUM_PADS && step >= 0 && step < MAX_STEPS)
        steps[pad][step].active = active;
}

void Pattern::setStepVelocity (int pad, int step, float vel)
{
    if (pad >= 0 && pad < NUM_PADS && step >= 0 && step < MAX_STEPS)
        steps[pad][step].velocity = juce::jlimit (0.0f, 1.0f, vel);
}

void Pattern::setStepProbability (int pad, int step, float prob)
{
    if (pad >= 0 && pad < NUM_PADS && step >= 0 && step < MAX_STEPS)
        steps[pad][step].probability = juce::jlimit (0.0f, 1.0f, prob);
}

void Pattern::clear()
{
    for (int p = 0; p < NUM_PADS; ++p)
        for (int s = 0; s < MAX_STEPS; ++s)
            steps[p][s] = Step();
}

void Pattern::clearPad (int pad)
{
    if (pad >= 0 && pad < NUM_PADS)
        for (int s = 0; s < MAX_STEPS; ++s)
            steps[pad][s] = Step();
}

Pattern Pattern::copy() const
{
    Pattern p;
    p.numSteps = numSteps;
    p.name = name + " (copy)";
    for (int pad = 0; pad < NUM_PADS; ++pad)
        for (int s = 0; s < MAX_STEPS; ++s)
            p.steps[pad][s] = steps[pad][s];
    return p;
}
