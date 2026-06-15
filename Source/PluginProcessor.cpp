#include "PluginProcessor.h"
#include "PluginEditor.h"

GrooveStationProcessor::GrooveStationProcessor()
    : AudioProcessor (BusesProperties()
                        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      stepSequencer (samplerEngine)
{
}

GrooveStationProcessor::~GrooveStationProcessor()
{
    stepSequencer.stop();
}

void GrooveStationProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    samplerEngine.prepare (sampleRate, samplesPerBlock);
}

void GrooveStationProcessor::releaseResources()
{
    stepSequencer.stop();
}

bool GrooveStationProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    return true;
}

void GrooveStationProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();
    stepSequencer.processPendingEvents();
    samplerEngine.processBlock (buffer, midiMessages);
}

juce::AudioProcessorEditor* GrooveStationProcessor::createEditor()
{
    return new GrooveStationEditor (*this);
}

void GrooveStationProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // Save state as XML
    auto xml = std::make_unique<juce::XmlElement> ("GrooveStationState");

    // Save pad states
    // Save current bank
    xml->setAttribute ("currentBank", samplerEngine.getCurrentBank());

    // Save all pad states across all banks
    for (int i = 0; i < SamplerEngine::TOTAL_PADS; ++i)
    {
        auto& state = samplerEngine.getPadState (i);
        auto* padXml = xml->createNewChildElement ("Pad");
        padXml->setAttribute ("index", i);
        padXml->setAttribute ("filePath", state.filePath);
        padXml->setAttribute ("volume", state.volume);
        padXml->setAttribute ("pan", state.pan);
        padXml->setAttribute ("pitch", state.pitch);
        padXml->setAttribute ("startPos", state.startPos);
        padXml->setAttribute ("endPos", state.endPos);
        padXml->setAttribute ("reverse", state.reverse);
        padXml->setAttribute ("mute", state.mute);
        padXml->setAttribute ("attack", state.adsr.attack);
        padXml->setAttribute ("decay", state.adsr.decay);
        padXml->setAttribute ("sustain", state.adsr.sustain);
        padXml->setAttribute ("release", state.adsr.release);

        // Effects chain parameters
        auto& fx = state.effects;
        padXml->setAttribute ("fxFilterCutoff", fx.filterCutoff);
        padXml->setAttribute ("fxFilterRes", fx.filterResonance);
        padXml->setAttribute ("fxFilterType", fx.filterType);
        padXml->setAttribute ("fxReverbMix", fx.reverbMix);
        padXml->setAttribute ("fxReverbRoom", fx.reverbRoom);
        padXml->setAttribute ("fxReverbDamp", fx.reverbDamp);
        padXml->setAttribute ("fxDelayMix", fx.delayMix);
        padXml->setAttribute ("fxDelayTime", fx.delayTime);
        padXml->setAttribute ("fxDelayFB", fx.delayFeedback);
        padXml->setAttribute ("fxDistAmt", fx.distortionAmount);
        padXml->setAttribute ("fxDistMix", fx.distortionMix);
        padXml->setAttribute ("fxEqLow", fx.eqLowGain);
        padXml->setAttribute ("fxEqMid", fx.eqMidGain);
        padXml->setAttribute ("fxEqHigh", fx.eqHighGain);
    }

    // Save master effects
    auto masterFx = samplerEngine.getMasterEffects().getParameters();
    auto* masterFxXml = xml->createNewChildElement ("MasterEffects");
    masterFxXml->setAttribute ("filterCutoff", masterFx.filterCutoff);
    masterFxXml->setAttribute ("filterRes", masterFx.filterResonance);
    masterFxXml->setAttribute ("filterType", masterFx.filterType);
    masterFxXml->setAttribute ("reverbMix", masterFx.reverbMix);
    masterFxXml->setAttribute ("reverbRoom", masterFx.reverbRoom);
    masterFxXml->setAttribute ("reverbDamp", masterFx.reverbDamp);
    masterFxXml->setAttribute ("delayMix", masterFx.delayMix);
    masterFxXml->setAttribute ("delayTime", masterFx.delayTime);
    masterFxXml->setAttribute ("delayFB", masterFx.delayFeedback);
    masterFxXml->setAttribute ("distAmt", masterFx.distortionAmount);
    masterFxXml->setAttribute ("distMix", masterFx.distortionMix);
    masterFxXml->setAttribute ("eqLow", masterFx.eqLowGain);
    masterFxXml->setAttribute ("eqMid", masterFx.eqMidGain);
    masterFxXml->setAttribute ("eqHigh", masterFx.eqHighGain);

    // Save sequencer state
    auto* seqXml = xml->createNewChildElement ("Sequencer");
    seqXml->setAttribute ("bpm", stepSequencer.getBPM());
    seqXml->setAttribute ("swing", stepSequencer.getSwing());
    seqXml->setAttribute ("masterVolume", samplerEngine.getMasterVolume());

    // Save patterns
    for (int p = 0; p < StepSequencer::MAX_PATTERNS; ++p)
    {
        auto& pattern = stepSequencer.getPattern (p);
        auto* patXml = seqXml->createNewChildElement ("Pattern");
        patXml->setAttribute ("index", p);
        patXml->setAttribute ("numSteps", pattern.getNumSteps());

        for (int pad = 0; pad < Pattern::NUM_PADS; ++pad)
        {
            for (int step = 0; step < pattern.getNumSteps(); ++step)
            {
                auto& s = pattern.getStep (pad, step);
                if (s.active)
                {
                    auto* stepXml = patXml->createNewChildElement ("Step");
                    stepXml->setAttribute ("pad", pad);
                    stepXml->setAttribute ("step", step);
                    stepXml->setAttribute ("velocity", s.velocity);
                    stepXml->setAttribute ("probability", s.probability);
                }
            }
        }
    }

    copyXmlToBinary (*xml, destData);
}

void GrooveStationProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto xml = getXmlFromBinary (data, sizeInBytes);
    if (xml == nullptr || ! xml->hasTagName ("GrooveStationState"))
        return;

    // Restore pads
    for (auto* padXml : xml->getChildWithTagNameIterator ("Pad"))
    {
        int index = padXml->getIntAttribute ("index", -1);
        if (index < 0 || index >= SamplerEngine::TOTAL_PADS) continue;

        juce::String filePath = padXml->getStringAttribute ("filePath");
        if (filePath.isNotEmpty())
        {
            juce::File file (filePath);
            if (file.existsAsFile())
                samplerEngine.loadSample (index, file);
        }

        auto& state = samplerEngine.getPadState (index);
        state.volume   = (float) padXml->getDoubleAttribute ("volume", 0.0);
        state.pan      = (float) padXml->getDoubleAttribute ("pan", 0.0);
        state.pitch    = (float) padXml->getDoubleAttribute ("pitch", 0.0);
        state.startPos = (float) padXml->getDoubleAttribute ("startPos", 0.0);
        state.endPos   = (float) padXml->getDoubleAttribute ("endPos", 1.0);
        state.reverse  = padXml->getBoolAttribute ("reverse", false);
        state.mute     = padXml->getBoolAttribute ("mute", false);
        state.adsr.attack  = (float) padXml->getDoubleAttribute ("attack", 0.005);
        state.adsr.decay   = (float) padXml->getDoubleAttribute ("decay", 0.1);
        state.adsr.sustain = (float) padXml->getDoubleAttribute ("sustain", 0.8);
        state.adsr.release = (float) padXml->getDoubleAttribute ("release", 0.3);

        // Restore effects chain parameters
        auto& fx = state.effects;
        fx.filterCutoff     = (float) padXml->getDoubleAttribute ("fxFilterCutoff", 20000.0);
        fx.filterResonance  = (float) padXml->getDoubleAttribute ("fxFilterRes", 0.707);
        fx.filterType       = padXml->getIntAttribute ("fxFilterType", 0);
        fx.reverbMix        = (float) padXml->getDoubleAttribute ("fxReverbMix", 0.0);
        fx.reverbRoom       = (float) padXml->getDoubleAttribute ("fxReverbRoom", 0.5);
        fx.reverbDamp       = (float) padXml->getDoubleAttribute ("fxReverbDamp", 0.5);
        fx.delayMix         = (float) padXml->getDoubleAttribute ("fxDelayMix", 0.0);
        fx.delayTime        = (float) padXml->getDoubleAttribute ("fxDelayTime", 0.3);
        fx.delayFeedback    = (float) padXml->getDoubleAttribute ("fxDelayFB", 0.4);
        fx.distortionAmount = (float) padXml->getDoubleAttribute ("fxDistAmt", 0.0);
        fx.distortionMix    = (float) padXml->getDoubleAttribute ("fxDistMix", 0.0);
        fx.eqLowGain        = (float) padXml->getDoubleAttribute ("fxEqLow", 0.0);
        fx.eqMidGain        = (float) padXml->getDoubleAttribute ("fxEqMid", 0.0);
        fx.eqHighGain       = (float) padXml->getDoubleAttribute ("fxEqHigh", 0.0);

        samplerEngine.updatePadParameters (index);
    }

    // Restore master effects
    if (auto* masterFxXml = xml->getChildByName ("MasterEffects"))
    {
        EffectsChain::Parameters mfx;
        mfx.filterCutoff     = (float) masterFxXml->getDoubleAttribute ("filterCutoff", 20000.0);
        mfx.filterResonance  = (float) masterFxXml->getDoubleAttribute ("filterRes", 0.707);
        mfx.filterType       = masterFxXml->getIntAttribute ("filterType", 0);
        mfx.reverbMix        = (float) masterFxXml->getDoubleAttribute ("reverbMix", 0.0);
        mfx.reverbRoom       = (float) masterFxXml->getDoubleAttribute ("reverbRoom", 0.5);
        mfx.reverbDamp       = (float) masterFxXml->getDoubleAttribute ("reverbDamp", 0.5);
        mfx.delayMix         = (float) masterFxXml->getDoubleAttribute ("delayMix", 0.0);
        mfx.delayTime        = (float) masterFxXml->getDoubleAttribute ("delayTime", 0.3);
        mfx.delayFeedback    = (float) masterFxXml->getDoubleAttribute ("delayFB", 0.4);
        mfx.distortionAmount = (float) masterFxXml->getDoubleAttribute ("distAmt", 0.0);
        mfx.distortionMix    = (float) masterFxXml->getDoubleAttribute ("distMix", 0.0);
        mfx.eqLowGain        = (float) masterFxXml->getDoubleAttribute ("eqLow", 0.0);
        mfx.eqMidGain        = (float) masterFxXml->getDoubleAttribute ("eqMid", 0.0);
        mfx.eqHighGain       = (float) masterFxXml->getDoubleAttribute ("eqHigh", 0.0);
        samplerEngine.getMasterEffects().setParameters (mfx);
    }

    // Restore bank
    samplerEngine.setCurrentBank (xml->getIntAttribute ("currentBank", 0));

    // Restore sequencer
    if (auto* seqXml = xml->getChildByName ("Sequencer"))
    {
        stepSequencer.setBPM (seqXml->getDoubleAttribute ("bpm", 120.0));
        stepSequencer.setSwing ((float) seqXml->getDoubleAttribute ("swing", 0.0));
        samplerEngine.setMasterVolume ((float) seqXml->getDoubleAttribute ("masterVolume", 0.0));

        for (auto* patXml : seqXml->getChildWithTagNameIterator ("Pattern"))
        {
            int index = patXml->getIntAttribute ("index", -1);
            if (index < 0 || index >= StepSequencer::MAX_PATTERNS) continue;

            auto& pattern = stepSequencer.getPattern (index);
            pattern.setNumSteps (patXml->getIntAttribute ("numSteps", 16));

            for (auto* stepXml : patXml->getChildWithTagNameIterator ("Step"))
            {
                int pad = stepXml->getIntAttribute ("pad", -1);
                int step = stepXml->getIntAttribute ("step", -1);
                if (pad < 0 || step < 0) continue;

                pattern.setStepActive (pad, step, true);
                pattern.setStepVelocity (pad, step, (float) stepXml->getDoubleAttribute ("velocity", 1.0));
                pattern.setStepProbability (pad, step, (float) stepXml->getDoubleAttribute ("probability", 1.0));
            }
        }
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new GrooveStationProcessor();
}
