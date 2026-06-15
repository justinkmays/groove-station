#pragma once
#include <JuceHeader.h>

class EffectsChain
{
public:
    struct Parameters
    {
        // Filter
        float filterCutoff    = 20000.0f; // Hz
        float filterResonance = 0.707f;
        int   filterType      = 0; // 0=lowpass, 1=highpass, 2=bandpass

        // Reverb
        float reverbMix    = 0.0f;
        float reverbRoom   = 0.5f;
        float reverbDamp   = 0.5f;

        // Delay
        float delayMix     = 0.0f;
        float delayTime    = 0.3f; // seconds
        float delayFeedback = 0.4f;

        // Distortion
        float distortionAmount = 0.0f;
        float distortionMix    = 0.0f;

        // EQ
        float eqLowGain  = 0.0f;  // dB
        float eqMidGain  = 0.0f;
        float eqHighGain = 0.0f;
    };

    void prepare (double sampleRate, int samplesPerBlock);
    void process (juce::AudioBuffer<float>& buffer);
    void setParameters (const Parameters& p);
    Parameters getParameters() const { return params; }
    void reset();

private:
    Parameters params;
    double currentSampleRate = 44100.0;
    int currentBlockSize = 512;

    // Filter
    juce::dsp::StateVariableTPTFilter<float> filter;

    // Reverb
    juce::Reverb reverb;
    juce::Reverb::Parameters reverbParams;

    // Delay line
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> delayLine { 192000 };
    float delaySmoothed = 0.0f;

    // Distortion waveshaper
    juce::dsp::WaveShaper<float> waveshaper;

    // EQ filters
    juce::dsp::IIR::Filter<float> eqLow, eqMid, eqHigh;

    void updateFilter();
    void updateEQ();
};
