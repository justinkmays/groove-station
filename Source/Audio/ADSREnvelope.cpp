#include "ADSREnvelope.h"

void ADSREnvelope::setSampleRate (double sr)
{
    sampleRate = sr;
    recalculate();
}

void ADSREnvelope::setParameters (const Parameters& p)
{
    params = p;
    recalculate();
}

void ADSREnvelope::noteOn()
{
    currentLevel = 0.0f;
    state = State::Attack;
}

void ADSREnvelope::noteOff()
{
    if (state != State::Idle)
        state = State::Release;
}

void ADSREnvelope::reset()
{
    state = State::Idle;
    currentLevel = 0.0f;
}

bool ADSREnvelope::isActive() const
{
    return state != State::Idle;
}

float ADSREnvelope::getNextSample()
{
    switch (state)
    {
        case State::Idle:
            return 0.0f;

        case State::Attack:
            currentLevel += attackDelta;
            if (currentLevel >= 1.0f)
            {
                currentLevel = 1.0f;
                state = State::Decay;
            }
            break;

        case State::Decay:
            currentLevel -= decayDelta;
            if (currentLevel <= params.sustain)
            {
                currentLevel = params.sustain;
                state = State::Sustain;
            }
            break;

        case State::Sustain:
            currentLevel = params.sustain;
            break;

        case State::Release:
            currentLevel -= releaseDelta;
            if (currentLevel <= 0.0f)
            {
                currentLevel = 0.0f;
                state = State::Idle;
            }
            break;
    }
    return currentLevel;
}

void ADSREnvelope::recalculate()
{
    float attackSamples  = std::max (1.0f, (float)(params.attack  * sampleRate));
    float decaySamples   = std::max (1.0f, (float)(params.decay   * sampleRate));
    float releaseSamples = std::max (1.0f, (float)(params.release * sampleRate));

    attackDelta  = 1.0f / attackSamples;
    decayDelta   = (1.0f - params.sustain) / decaySamples;
    releaseDelta = params.sustain / releaseSamples;
}
