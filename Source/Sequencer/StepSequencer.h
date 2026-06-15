#pragma once
#include <JuceHeader.h>
#include "Pattern.h"
#include "../Audio/SamplerEngine.h"

class StepSequencer : public juce::HighResolutionTimer
{
public:
    static constexpr int MAX_PATTERNS = 16;
    static constexpr int EVENT_FIFO_SIZE = 256;

    struct PadEvent
    {
        int padIndex = -1;      // absolute pad index
        float velocity = 0.0f;
    };

    StepSequencer (SamplerEngine& engine);
    ~StepSequencer() override;

    // Transport
    void play();
    void stop();
    void pause();
    bool isPlaying() const { return playing; }

    // Tempo
    void setBPM (double bpm);
    double getBPM() const { return currentBPM; }
    void setSwing (float amount);
    float getSwing() const { return swingAmount; }

    // Time signature
    void setTimeSignature (int numerator, int denominator);
    int getTimeSigNumerator() const { return timeSigNum; }
    int getTimeSigDenominator() const { return timeSigDen; }

    // Pattern management
    Pattern& getCurrentPattern();
    Pattern& getPattern (int index);
    int getCurrentPatternIndex() const { return currentPatternIdx; }
    void setCurrentPattern (int index);
    void copyPattern (int src, int dst);
    void clearPattern (int index);

    // Step info
    int getCurrentStep() const { return currentStep; }
    int getCurrentBar() const { return currentBar; }

    // Quantization
    enum class Quantize { Q_1_4, Q_1_8, Q_1_16, Q_1_32 };
    void setQuantize (Quantize q) { quantize = q; updateTimerInterval(); }
    Quantize getQuantize() const { return quantize; }

    // Lock-free event consumption (called from audio thread)
    void processPendingEvents();

    // Callback for UI updates
    std::function<void (int step)> onStepChanged;

    void hiResTimerCallback() override;

private:
    SamplerEngine& samplerEngine;

    Pattern patterns[MAX_PATTERNS];
    int currentPatternIdx = 0;
    int currentStep = 0;
    int currentBar = 0;

    double currentBPM = 120.0;
    float swingAmount = 0.0f; // 0..1
    int timeSigNum = 4;
    int timeSigDen = 4;
    Quantize quantize = Quantize::Q_1_16;

    bool playing = false;
    juce::Random random;

    // Lock-free FIFO for timer→audio thread communication
    juce::AbstractFifo eventFifo { EVENT_FIFO_SIZE };
    PadEvent eventBuffer[EVENT_FIFO_SIZE];

    void advanceStep();
    void updateTimerInterval();
    double getStepIntervalMs() const;
};
