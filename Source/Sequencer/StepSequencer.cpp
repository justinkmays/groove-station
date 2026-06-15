#include "StepSequencer.h"

StepSequencer::StepSequencer (SamplerEngine& engine)
    : samplerEngine (engine)
{
    for (int i = 0; i < MAX_PATTERNS; ++i)
        patterns[i].setName ("Pattern " + juce::String (i + 1));
}

StepSequencer::~StepSequencer()
{
    stopTimer();
}

void StepSequencer::play()
{
    if (! playing)
    {
        currentStep = 0;
        currentBar = 0;
    }
    playing = true;
    updateTimerInterval();
}

void StepSequencer::stop()
{
    playing = false;
    stopTimer();
    currentStep = 0;
    currentBar = 0;
}

void StepSequencer::pause()
{
    playing = false;
    stopTimer();
}

void StepSequencer::setBPM (double bpm)
{
    currentBPM = juce::jlimit (30.0, 300.0, bpm);
    if (playing)
        updateTimerInterval();
}

void StepSequencer::setSwing (float amount)
{
    swingAmount = juce::jlimit (0.0f, 1.0f, amount);
}

void StepSequencer::setTimeSignature (int numerator, int denominator)
{
    timeSigNum = juce::jlimit (1, 16, numerator);
    timeSigDen = juce::jlimit (1, 16, denominator);
}

Pattern& StepSequencer::getCurrentPattern()
{
    return patterns[currentPatternIdx];
}

Pattern& StepSequencer::getPattern (int index)
{
    return patterns[juce::jlimit (0, MAX_PATTERNS - 1, index)];
}

void StepSequencer::setCurrentPattern (int index)
{
    currentPatternIdx = juce::jlimit (0, MAX_PATTERNS - 1, index);
}

void StepSequencer::copyPattern (int src, int dst)
{
    if (src >= 0 && src < MAX_PATTERNS && dst >= 0 && dst < MAX_PATTERNS)
        patterns[dst] = patterns[src].copy();
}

void StepSequencer::clearPattern (int index)
{
    if (index >= 0 && index < MAX_PATTERNS)
        patterns[index].clear();
}

void StepSequencer::hiResTimerCallback()
{
    if (! playing)
        return;

    advanceStep();
}

void StepSequencer::advanceStep()
{
    auto& pattern = getCurrentPattern();
    int numSteps = pattern.getNumSteps();

    // Queue active steps into lock-free FIFO for audio thread consumption
    for (int pad = 0; pad < Pattern::NUM_PADS; ++pad)
    {
        auto& step = pattern.getStep (pad, currentStep);
        if (step.active)
        {
            if (step.probability >= 1.0f || random.nextFloat() < step.probability)
            {
                int absPad = samplerEngine.absolutePadIndex (pad);

                const auto scope = eventFifo.write (1);
                if (scope.blockSize1 > 0)
                    eventBuffer[scope.startIndex1] = { absPad, step.velocity };
                else if (scope.blockSize2 > 0)
                    eventBuffer[scope.startIndex2] = { absPad, step.velocity };
            }
        }
    }

    // Notify UI
    if (onStepChanged)
        onStepChanged (currentStep);

    // Advance
    currentStep++;
    if (currentStep >= numSteps)
    {
        currentStep = 0;
        currentBar++;
    }

    // Apply swing to next step timing
    if (playing)
        updateTimerInterval();
}

void StepSequencer::processPendingEvents()
{
    const auto scope = eventFifo.read (eventFifo.getNumReady());

    for (int i = 0; i < scope.blockSize1; ++i)
    {
        auto& evt = eventBuffer[scope.startIndex1 + i];
        if (evt.padIndex >= 0)
            samplerEngine.triggerPad (evt.padIndex, evt.velocity);
    }

    for (int i = 0; i < scope.blockSize2; ++i)
    {
        auto& evt = eventBuffer[scope.startIndex2 + i];
        if (evt.padIndex >= 0)
            samplerEngine.triggerPad (evt.padIndex, evt.velocity);
    }
}

void StepSequencer::updateTimerInterval()
{
    double intervalMs = getStepIntervalMs();

    // Apply swing: even steps are on time, odd steps are delayed
    if (swingAmount > 0.0f && (currentStep % 2 == 1))
    {
        intervalMs *= (1.0f + swingAmount * 0.5f);
    }
    else if (swingAmount > 0.0f && (currentStep % 2 == 0))
    {
        intervalMs *= (1.0f - swingAmount * 0.25f);
    }

    startTimer ((int) std::max (1.0, intervalMs));
}

double StepSequencer::getStepIntervalMs() const
{
    double beatsPerSecond = currentBPM / 60.0;
    double stepsPerBeat = 1.0;

    switch (quantize)
    {
        case Quantize::Q_1_4:  stepsPerBeat = 1.0; break;
        case Quantize::Q_1_8:  stepsPerBeat = 2.0; break;
        case Quantize::Q_1_16: stepsPerBeat = 4.0; break;
        case Quantize::Q_1_32: stepsPerBeat = 8.0; break;
    }

    double stepsPerSecond = beatsPerSecond * stepsPerBeat;
    return 1000.0 / stepsPerSecond;
}
