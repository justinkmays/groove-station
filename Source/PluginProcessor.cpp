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
    }

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

        samplerEngine.updatePadParameters (index);
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
