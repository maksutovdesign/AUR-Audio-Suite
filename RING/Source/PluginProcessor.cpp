#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Presets.h"

RingProcessor::RingProcessor()
    : AudioProcessor (BusesProperties()
        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMETERS", Params::createLayout())
{
    pFreq   = apvts.getRawParameterValue (ParamID::freq);
    pMix    = apvts.getRawParameterValue (ParamID::mix);
    pOutput = apvts.getRawParameterValue (ParamID::output);
    pBypass = apvts.getRawParameterValue (ParamID::bypass);
}

void RingProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    ring.prepare (sampleRate);
    outGain.reset (sampleRate, 0.02);
    bypassMix.reset (sampleRate, 0.01);
    dryBuffer.setSize (juce::jmax (1, getTotalNumOutputChannels()), samplesPerBlock);
    setLatencySamples (0);
}

bool RingProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& out = layouts.getMainOutputChannelSet();
    if (out != juce::AudioChannelSet::mono() && out != juce::AudioChannelSet::stereo())
        return false;
    return layouts.getMainInputChannelSet() == out;
}

void RingProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const int totalIn  = getTotalNumInputChannels();
    const int totalOut = getTotalNumOutputChannels();
    const int n        = buffer.getNumSamples();
    for (int ch = totalIn; ch < totalOut; ++ch)
        buffer.clear (ch, 0, n);

    meters.pushInputPeak (buffer.getMagnitude (0, n));

    const bool bypassed = *pBypass > 0.5f;
    bypassMix.setTargetValue (bypassed ? 1.0f : 0.0f);
    dryBuffer.makeCopyOf (buffer, true);

    ring.setParameters (pFreq->load(), pMix->load());
    ring.process (buffer.getArrayOfWritePointers(), totalOut, n);

    outGain.setTargetValue (juce::Decibels::decibelsToGain (pOutput->load()));
    for (int s = 0; s < n; ++s)
    {
        const float g = outGain.getNextValue();
        for (int ch = 0; ch < totalOut; ++ch)
            buffer.getWritePointer (ch)[s] *= g;
    }

    if (bypassMix.isSmoothing() || bypassed)
    {
        for (int s = 0; s < n; ++s)
        {
            const float dryAmt = bypassMix.getNextValue();
            const float wetAmt = 1.0f - dryAmt;
            for (int ch = 0; ch < totalOut; ++ch)
            {
                auto* w = buffer.getWritePointer (ch);
                const auto* d = dryBuffer.getReadPointer (ch);
                w[s] = w[s] * wetAmt + d[s] * dryAmt;
            }
        }
    }

    meters.pushOutputPeak (buffer.getMagnitude (0, n));
}

juce::AudioProcessorEditor* RingProcessor::createEditor() { return new RingEditor (*this); }

int RingProcessor::getNumPrograms() { return (int) Presets::getFactoryPresets().size(); }

void RingProcessor::setCurrentProgram (int index)
{
    if (index < 0 || index >= getNumPrograms()) return;
    currentProgram = index;
    Presets::apply (index, apvts);
}

const juce::String RingProcessor::getProgramName (int index)
{
    const auto& p = Presets::getFactoryPresets();
    return (index >= 0 && index < (int) p.size()) ? p[(size_t) index].name : juce::String{};
}

void RingProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    if (auto state = apvts.copyState(); state.isValid())
    {
        juce::MemoryOutputStream mos (dest, false);
        state.writeToStream (mos);
    }
}

void RingProcessor::setStateInformation (const void* data, int size)
{
    auto tree = juce::ValueTree::readFromData (data, (size_t) size);
    if (tree.isValid()) apvts.replaceState (tree);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new RingProcessor(); }
