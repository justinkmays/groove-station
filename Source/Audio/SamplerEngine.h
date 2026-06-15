#pragma once
#include <JuceHeader.h>
#include "SampleVoice.h"
#include "EffectsChain.h"

// Manages 16 pads, each with its own sample, voice pool, and effects
class SamplerEngine
{
public:
    static constexpr int NUM_PADS = 16;
    static constexpr int VOICES_PER_PAD = 4;
    static constexpr int BASE_MIDI_NOTE = 36; // C2 = pad 0

    struct PadState
    {
        juce::String sampleName;
        juce::String filePath;
        float volume      = 0.0f;  // dB
        float pan          = 0.0f;  // -1..1
        float pitch        = 0.0f;  // semitones
        float startPos     = 0.0f;
        float endPos       = 1.0f;
        bool  reverse      = false;
        bool  mute         = false;
        bool  solo         = false;
        ADSREnvelope::Parameters adsr;
        EffectsChain::Parameters effects;
    };

    SamplerEngine();
    ~SamplerEngine();

    void prepare (double sampleRate, int samplesPerBlock);
    void processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages);

    bool loadSample (int padIndex, const juce::File& file);
    void clearSample (int padIndex);
    void triggerPad (int padIndex, float velocity = 1.0f);
    void releasePad (int padIndex);

    PadState& getPadState (int padIndex) { return padStates[padIndex]; }
    const PadState& getPadState (int padIndex) const { return padStates[padIndex]; }
    void updatePadParameters (int padIndex);

    bool hasSample (int padIndex) const;
    juce::AudioBuffer<float>* getSampleBuffer (int padIndex);
    double getSampleRate (int padIndex) const;
    int getSampleLengthSamples (int padIndex) const;

    float getMasterVolume() const { return masterVolume; }
    void setMasterVolume (float db) { masterVolume = db; }

    EffectsChain& getMasterEffects() { return masterEffects; }

private:
    juce::Synthesiser synthesisers[NUM_PADS];
    PadState padStates[NUM_PADS];
    EffectsChain padEffects[NUM_PADS];
    EffectsChain masterEffects;

    juce::AudioFormatManager formatManager;
    double currentSampleRate = 44100.0;
    int currentBlockSize = 512;
    float masterVolume = 0.0f; // dB

    juce::AudioBuffer<float> padBuffer;
};
