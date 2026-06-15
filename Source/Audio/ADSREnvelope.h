#pragma once
#include <JuceHeader.h>

class ADSREnvelope
{
public:
    struct Parameters
    {
        float attack  = 0.005f;  // seconds
        float decay   = 0.1f;
        float sustain = 0.8f;    // 0..1
        float release = 0.3f;
    };

    void setSampleRate (double sr);
    void setParameters (const Parameters& p);
    Parameters getParameters() const { return params; }

    void noteOn();
    void noteOff();
    void reset();
    float getNextSample();
    bool isActive() const;

private:
    enum class State { Idle, Attack, Decay, Sustain, Release };
    State state = State::Idle;
    Parameters params;
    double sampleRate = 44100.0;
    float currentLevel = 0.0f;
    float attackDelta = 0.0f, decayDelta = 0.0f, releaseDelta = 0.0f;

    void recalculate();
};
