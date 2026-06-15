#pragma once
#include <JuceHeader.h>
#include "ADSREnvelope.h"

// A single sample sound that can be loaded from a file
class SampleSound : public juce::SynthesiserSound
{
public:
    SampleSound (const juce::String& name, juce::AudioFormatReader& source,
                 int midiNote, double maxLengthSeconds = 30.0);

    bool appliesToNote (int midiNoteNumber) override { return midiNoteNumber == assignedMidiNote; }
    bool appliesToChannel (int) override { return true; }

    juce::AudioBuffer<float>& getBuffer() { return buffer; }
    double getSampleRate() const { return sourceSampleRate; }
    int getAssignedMidiNote() const { return assignedMidiNote; }
    juce::String getName() const { return sampleName; }
    int getLengthInSamples() const { return buffer.getNumSamples(); }

private:
    juce::String sampleName;
    juce::AudioBuffer<float> buffer;
    double sourceSampleRate;
    int assignedMidiNote;
};

// A voice that plays a SampleSound with ADSR, pitch, pan, volume
class SampleVoice : public juce::SynthesiserVoice
{
public:
    SampleVoice();

    bool canPlaySound (juce::SynthesiserSound*) override;
    void startNote (int midiNoteNumber, float velocity,
                    juce::SynthesiserSound*, int pitchWheel) override;
    void stopNote (float velocity, bool allowTailOff) override;
    void pitchWheelMoved (int) override {}
    void controllerMoved (int, int) override {}
    void renderNextBlock (juce::AudioBuffer<float>&, int startSample, int numSamples) override;

    void setADSR (const ADSREnvelope::Parameters& p) { envelope.setParameters (p); }
    void setPitch (float semitones) { pitchShift = semitones; }
    void setPan (float p) { pan = juce::jlimit (-1.0f, 1.0f, p); }
    void setVolumeDB (float db) { volume = juce::Decibels::decibelsToGain (db); }
    void setReverse (bool r) { reverse = r; }
    void setStartPosition (float normalized) { startPos = juce::jlimit (0.0f, 1.0f, normalized); }
    void setEndPosition (float normalized) { endPos = juce::jlimit (0.0f, 1.0f, normalized); }

    ADSREnvelope::Parameters getADSRParams() const { return envelope.getParameters(); }

private:
    ADSREnvelope envelope;
    double samplePosition = 0.0;
    float pitchShift = 0.0f;
    float pan = 0.0f;
    float volume = 1.0f;
    float velocity = 0.0f;
    bool reverse = false;
    float startPos = 0.0f;
    float endPos = 1.0f;
    SampleSound* currentSound = nullptr;
};
