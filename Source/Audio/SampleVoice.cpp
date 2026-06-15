#include "SampleVoice.h"

// ============== SampleSound ==============

SampleSound::SampleSound (const juce::String& name, juce::AudioFormatReader& source,
                           int midiNote, double maxLengthSeconds)
    : sampleName (name),
      sourceSampleRate (source.sampleRate),
      assignedMidiNote (midiNote)
{
    int maxSamples = (int)(source.sampleRate * maxLengthSeconds);
    int length = (int) std::min ((juce::int64) maxSamples, source.lengthInSamples);

    buffer.setSize ((int) source.numChannels, length);
    source.read (&buffer, 0, length, 0, true, true);
}

// ============== SampleVoice ==============

SampleVoice::SampleVoice()
{
}

bool SampleVoice::canPlaySound (juce::SynthesiserSound* sound)
{
    return dynamic_cast<SampleSound*> (sound) != nullptr;
}

void SampleVoice::startNote (int /*midiNoteNumber*/, float vel,
                              juce::SynthesiserSound* sound, int /*pitchWheel*/)
{
    currentSound = dynamic_cast<SampleSound*> (sound);
    if (currentSound == nullptr)
        return;

    velocity = vel;
    envelope.setSampleRate (getSampleRate());
    envelope.noteOn();

    int totalSamples = currentSound->getLengthInSamples();
    if (reverse)
        samplePosition = (double)(totalSamples - 1) * endPos;
    else
        samplePosition = (double)(totalSamples) * startPos;
}

void SampleVoice::stopNote (float /*velocity*/, bool allowTailOff)
{
    if (allowTailOff)
    {
        envelope.noteOff();
    }
    else
    {
        envelope.reset();
        clearCurrentNote();
        currentSound = nullptr;
    }
}

void SampleVoice::renderNextBlock (juce::AudioBuffer<float>& outputBuffer,
                                    int startSample, int numSamples)
{
    if (currentSound == nullptr)
        return;

    auto& sampleBuffer = currentSound->getBuffer();
    int totalSamples = sampleBuffer.getNumSamples();
    int numChannels = sampleBuffer.getNumChannels();

    double pitchRatio = std::pow (2.0, pitchShift / 12.0)
                        * (currentSound->getSampleRate() / getSampleRate());

    int sampleStart = (int)((double) totalSamples * startPos);
    int sampleEnd   = (int)((double) totalSamples * endPos);
    if (sampleEnd <= sampleStart) sampleEnd = totalSamples;

    float panL = std::cos (juce::MathConstants<float>::halfPi * (pan + 1.0f) * 0.5f);
    float panR = std::sin (juce::MathConstants<float>::halfPi * (pan + 1.0f) * 0.5f);

    for (int i = 0; i < numSamples; ++i)
    {
        float envLevel = envelope.getNextSample();

        if (! envelope.isActive() && envLevel <= 0.0f)
        {
            clearCurrentNote();
            currentSound = nullptr;
            break;
        }

        // Check bounds
        bool outOfBounds = reverse
            ? (samplePosition < sampleStart)
            : (samplePosition >= sampleEnd);

        if (outOfBounds)
        {
            envelope.noteOff();
            if (! envelope.isActive())
            {
                clearCurrentNote();
                currentSound = nullptr;
                break;
            }
            // Continue with envelope tail but no new audio
            for (int ch = 0; ch < outputBuffer.getNumChannels(); ++ch)
                outputBuffer.addSample (ch, startSample + i, 0.0f);
            continue;
        }

        // Linear interpolation
        int pos0 = (int) samplePosition;
        int pos1 = pos0 + 1;
        pos0 = juce::jlimit (0, totalSamples - 1, pos0);
        pos1 = juce::jlimit (0, totalSamples - 1, pos1);
        float frac = (float)(samplePosition - (int) samplePosition);

        float sampleL = 0.0f, sampleR = 0.0f;

        if (numChannels >= 1)
        {
            auto* ch0 = sampleBuffer.getReadPointer (0);
            sampleL = ch0[pos0] + (ch0[pos1] - ch0[pos0]) * frac;
        }
        if (numChannels >= 2)
        {
            auto* ch1 = sampleBuffer.getReadPointer (1);
            sampleR = ch1[pos0] + (ch1[pos1] - ch1[pos0]) * frac;
        }
        else
        {
            sampleR = sampleL;
        }

        float gain = envLevel * velocity * volume;
        sampleL *= gain * panL;
        sampleR *= gain * panR;

        if (outputBuffer.getNumChannels() >= 1)
            outputBuffer.addSample (0, startSample + i, sampleL);
        if (outputBuffer.getNumChannels() >= 2)
            outputBuffer.addSample (1, startSample + i, sampleR);

        samplePosition += reverse ? -pitchRatio : pitchRatio;
    }
}
