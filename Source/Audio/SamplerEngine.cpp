#include "SamplerEngine.h"

SamplerEngine::SamplerEngine()
{
    formatManager.registerBasicFormats();

    for (int pad = 0; pad < NUM_PADS; ++pad)
    {
        for (int v = 0; v < VOICES_PER_PAD; ++v)
            synthesisers[pad].addVoice (new SampleVoice());
    }
}

SamplerEngine::~SamplerEngine() {}

void SamplerEngine::prepare (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    currentBlockSize = samplesPerBlock;

    for (int pad = 0; pad < NUM_PADS; ++pad)
    {
        synthesisers[pad].setCurrentPlaybackSampleRate (sampleRate);
        padEffects[pad].prepare (sampleRate, samplesPerBlock);
    }

    masterEffects.prepare (sampleRate, samplesPerBlock);
    padBuffer.setSize (2, samplesPerBlock);
}

bool SamplerEngine::loadSample (int padIndex, const juce::File& file)
{
    if (padIndex < 0 || padIndex >= NUM_PADS)
        return false;

    std::unique_ptr<juce::AudioFormatReader> reader (formatManager.createReaderFor (file));
    if (reader == nullptr)
        return false;

    int midiNote = BASE_MIDI_NOTE + padIndex;

    // Clear existing sounds
    synthesisers[padIndex].clearSounds();

    auto* sound = new SampleSound (file.getFileNameWithoutExtension(), *reader, midiNote);
    synthesisers[padIndex].addSound (sound);

    padStates[padIndex].sampleName = file.getFileNameWithoutExtension();
    padStates[padIndex].filePath = file.getFullPathName();
    padStates[padIndex].startPos = 0.0f;
    padStates[padIndex].endPos = 1.0f;

    updatePadParameters (padIndex);
    return true;
}

void SamplerEngine::clearSample (int padIndex)
{
    if (padIndex < 0 || padIndex >= NUM_PADS)
        return;

    synthesisers[padIndex].clearSounds();
    padStates[padIndex].sampleName.clear();
    padStates[padIndex].filePath.clear();
}

void SamplerEngine::triggerPad (int padIndex, float velocity)
{
    if (padIndex < 0 || padIndex >= NUM_PADS || ! hasSample (padIndex))
        return;

    if (padStates[padIndex].mute)
        return;

    int midiNote = BASE_MIDI_NOTE + padIndex;
    synthesisers[padIndex].noteOn (1, midiNote, velocity);
}

void SamplerEngine::releasePad (int padIndex)
{
    if (padIndex < 0 || padIndex >= NUM_PADS)
        return;

    int midiNote = BASE_MIDI_NOTE + padIndex;
    synthesisers[padIndex].noteOff (1, midiNote, 0.0f, true);
}

void SamplerEngine::updatePadParameters (int padIndex)
{
    if (padIndex < 0 || padIndex >= NUM_PADS)
        return;

    auto& state = padStates[padIndex];

    for (int v = 0; v < synthesisers[padIndex].getNumVoices(); ++v)
    {
        if (auto* voice = dynamic_cast<SampleVoice*> (synthesisers[padIndex].getVoice (v)))
        {
            voice->setADSR (state.adsr);
            voice->setPitch (state.pitch);
            voice->setPan (state.pan);
            voice->setVolumeDB (state.volume);
            voice->setReverse (state.reverse);
            voice->setStartPosition (state.startPos);
            voice->setEndPosition (state.endPos);
        }
    }

    padEffects[padIndex].setParameters (state.effects);
}

bool SamplerEngine::hasSample (int padIndex) const
{
    if (padIndex < 0 || padIndex >= NUM_PADS)
        return false;
    return synthesisers[padIndex].getNumSounds() > 0;
}

juce::AudioBuffer<float>* SamplerEngine::getSampleBuffer (int padIndex)
{
    if (padIndex < 0 || padIndex >= NUM_PADS)
        return nullptr;

    if (auto* sound = dynamic_cast<SampleSound*> (synthesisers[padIndex].getSound (0).get()))
        return &sound->getBuffer();

    return nullptr;
}

double SamplerEngine::getSampleRate (int padIndex) const
{
    if (padIndex < 0 || padIndex >= NUM_PADS)
        return currentSampleRate;

    if (auto* sound = dynamic_cast<SampleSound*> (synthesisers[padIndex].getSound (0).get()))
        return sound->getSampleRate();

    return currentSampleRate;
}

int SamplerEngine::getSampleLengthSamples (int padIndex) const
{
    if (padIndex < 0 || padIndex >= NUM_PADS)
        return 0;

    if (auto* sound = dynamic_cast<SampleSound*> (synthesisers[padIndex].getSound (0).get()))
        return sound->getLengthInSamples();

    return 0;
}

void SamplerEngine::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    buffer.clear();

    // Check for solo
    bool anySolo = false;
    for (int pad = 0; pad < NUM_PADS; ++pad)
        if (padStates[pad].solo) { anySolo = true; break; }

    for (int pad = 0; pad < NUM_PADS; ++pad)
    {
        if (! hasSample (pad)) continue;
        if (padStates[pad].mute) continue;
        if (anySolo && ! padStates[pad].solo) continue;

        // Render each pad into its own buffer
        padBuffer.setSize (buffer.getNumChannels(), buffer.getNumSamples(), false, false, true);
        padBuffer.clear();

        // Route MIDI for this pad
        juce::MidiBuffer padMidi;
        int midiNote = BASE_MIDI_NOTE + pad;

        for (const auto metadata : midiMessages)
        {
            auto msg = metadata.getMessage();
            if (msg.isNoteOnOrOff() && msg.getNoteNumber() == midiNote)
                padMidi.addEvent (msg, metadata.samplePosition);
        }

        synthesisers[pad].renderNextBlock (padBuffer, padMidi, 0, padBuffer.getNumSamples());

        // Per-pad effects
        padEffects[pad].process (padBuffer);

        // Mix into main buffer
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            buffer.addFrom (ch, 0, padBuffer, ch, 0, buffer.getNumSamples());
    }

    // Master effects
    masterEffects.process (buffer);

    // Master volume
    float masterGain = juce::Decibels::decibelsToGain (masterVolume);
    buffer.applyGain (masterGain);
}
