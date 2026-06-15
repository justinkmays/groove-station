#include "EffectsChain.h"

void EffectsChain::prepare (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    currentBlockSize = samplesPerBlock;

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = (juce::uint32) samplesPerBlock;
    spec.numChannels = 2;

    filter.prepare (spec);
    filter.setType (juce::dsp::StateVariableTPTFilterType::lowpass);

    delayLine.prepare (spec);
    delayLine.setMaximumDelayInSamples (192000);

    reverb.setSampleRate (sampleRate);

    eqLow.prepare (spec);
    eqMid.prepare (spec);
    eqHigh.prepare (spec);

    updateFilter();
    updateEQ();
}

void EffectsChain::setParameters (const Parameters& p)
{
    params = p;
    updateFilter();
    updateEQ();

    reverbParams.roomSize = p.reverbRoom;
    reverbParams.damping  = p.reverbDamp;
    reverbParams.wetLevel = p.reverbMix;
    reverbParams.dryLevel = 1.0f - p.reverbMix * 0.5f;
    reverb.setParameters (reverbParams);
}

void EffectsChain::reset()
{
    filter.reset();
    delayLine.reset();
    reverb.reset();
    eqLow.reset();
    eqMid.reset();
    eqHigh.reset();
}

void EffectsChain::process (juce::AudioBuffer<float>& buffer)
{
    int numSamples = buffer.getNumSamples();
    int numChannels = buffer.getNumChannels();

    // --- Filter ---
    {
        juce::dsp::AudioBlock<float> block (buffer);
        juce::dsp::ProcessContextReplacing<float> ctx (block);
        filter.process (ctx);
    }

    // --- Distortion ---
    if (params.distortionAmount > 0.01f)
    {
        float drive = 1.0f + params.distortionAmount * 20.0f;
        float mix = params.distortionMix;
        for (int ch = 0; ch < numChannels; ++ch)
        {
            auto* data = buffer.getWritePointer (ch);
            for (int i = 0; i < numSamples; ++i)
            {
                float dry = data[i];
                float wet = std::tanh (dry * drive) / std::sqrt (drive);
                data[i] = dry * (1.0f - mix) + wet * mix;
            }
        }
    }

    // --- Delay ---
    if (params.delayMix > 0.01f)
    {
        float delaySamples = params.delayTime * (float) currentSampleRate;
        float feedback = params.delayFeedback;
        float mix = params.delayMix;

        for (int ch = 0; ch < numChannels; ++ch)
        {
            auto* data = buffer.getWritePointer (ch);
            for (int i = 0; i < numSamples; ++i)
            {
                float delayed = delayLine.popSample (ch, delaySamples);
                float input = data[i] + delayed * feedback;
                delayLine.pushSample (ch, input);
                data[i] = data[i] * (1.0f - mix) + delayed * mix;
            }
        }
    }

    // --- Reverb ---
    if (params.reverbMix > 0.01f)
    {
        if (numChannels == 2)
            reverb.processStereo (buffer.getWritePointer (0), buffer.getWritePointer (1), numSamples);
        else if (numChannels == 1)
            reverb.processMono (buffer.getWritePointer (0), numSamples);
    }

    // --- EQ ---
    {
        juce::dsp::AudioBlock<float> block (buffer);
        for (int ch = 0; ch < numChannels; ++ch)
        {
            auto channelBlock = block.getSingleChannelBlock ((size_t) ch);
            juce::dsp::ProcessContextReplacing<float> ctx (channelBlock);
            eqLow.process (ctx);
            eqMid.process (ctx);
            eqHigh.process (ctx);
        }
    }
}

void EffectsChain::updateFilter()
{
    float freq = juce::jlimit (20.0f, 20000.0f, params.filterCutoff);
    float q    = juce::jlimit (0.1f, 10.0f, params.filterResonance);

    switch (params.filterType)
    {
        case 0: filter.setType (juce::dsp::StateVariableTPTFilterType::lowpass);  break;
        case 1: filter.setType (juce::dsp::StateVariableTPTFilterType::highpass); break;
        case 2: filter.setType (juce::dsp::StateVariableTPTFilterType::bandpass); break;
    }
    filter.setCutoffFrequency (freq);
    filter.setResonance (q);
}

void EffectsChain::updateEQ()
{
    auto lowCoeffs  = juce::dsp::IIR::Coefficients<float>::makeLowShelf  (currentSampleRate, 200.0f,  0.707f, juce::Decibels::decibelsToGain (params.eqLowGain));
    auto midCoeffs  = juce::dsp::IIR::Coefficients<float>::makePeakFilter (currentSampleRate, 1000.0f, 1.0f,   juce::Decibels::decibelsToGain (params.eqMidGain));
    auto highCoeffs = juce::dsp::IIR::Coefficients<float>::makeHighShelf (currentSampleRate, 5000.0f, 0.707f, juce::Decibels::decibelsToGain (params.eqHighGain));

    *eqLow.coefficients  = *lowCoeffs;
    *eqMid.coefficients  = *midCoeffs;
    *eqHigh.coefficients = *highCoeffs;
}
