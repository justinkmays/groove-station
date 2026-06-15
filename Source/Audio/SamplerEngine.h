#pragma once
#include <JuceHeader.h>
#include "SampleVoice.h"
#include "EffectsChain.h"

class SamplerEngine
{
public:
    static constexpr int PADS_PER_BANK = 16;
    static constexpr int NUM_BANKS = 4;
    static constexpr int TOTAL_PADS = PADS_PER_BANK * NUM_BANKS; // 64
    static constexpr int NUM_PADS = PADS_PER_BANK; // visible pads in current bank
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

    // Bank management
    int  getCurrentBank() const { return currentBank; }
    void setCurrentBank (int bank);
    juce::String getBankName (int bank) const;

    // Pad access (absolute index 0..63)
    bool loadSample (int absolutePadIndex, const juce::File& file);
    void clearSample (int absolutePadIndex);
    void triggerPad (int absolutePadIndex, float velocity = 1.0f);
    void releasePad (int absolutePadIndex);

    PadState& getPadState (int absolutePadIndex) { return padStates[absolutePadIndex]; }
    const PadState& getPadState (int absolutePadIndex) const { return padStates[absolutePadIndex]; }
    void updatePadParameters (int absolutePadIndex);

    bool hasSample (int absolutePadIndex) const;
    juce::AudioBuffer<float>* getSampleBuffer (int absolutePadIndex);
    double getSampleRate (int absolutePadIndex) const;
    int getSampleLengthSamples (int absolutePadIndex) const;

    // Convenience: bank-relative access
    int absolutePadIndex (int bankRelativePad) const { return currentBank * PADS_PER_BANK + bankRelativePad; }
    int absolutePadIndex (int bank, int pad) const { return bank * PADS_PER_BANK + pad; }

    float getMasterVolume() const { return masterVolume; }
    void setMasterVolume (float db) { masterVolume = db; }

    EffectsChain& getMasterEffects() { return masterEffects; }

    juce::AudioFormatManager& getFormatManager() { return formatManager; }

private:
    int currentBank = 0;

    juce::Synthesiser synthesisers[TOTAL_PADS];
    PadState padStates[TOTAL_PADS];
    EffectsChain padEffects[TOTAL_PADS];
    EffectsChain masterEffects;

    juce::AudioFormatManager formatManager;
    double currentSampleRate = 44100.0;
    int currentBlockSize = 512;
    float masterVolume = 0.0f; // dB

    juce::AudioBuffer<float> padBuffer;
};
