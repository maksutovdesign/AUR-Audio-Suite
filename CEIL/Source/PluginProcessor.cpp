#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Presets.h"

CeilProcessor::CeilProcessor()
    : AudioProcessor (BusesProperties()
        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMETERS", Params::createLayout())
{
    pGain    = apvts.getRawParameterValue (ParamID::gain);
    pCeiling = apvts.getRawParameterValue (ParamID::ceiling);
    pRelease = apvts.getRawParameterValue (ParamID::release);
    pBypass  = apvts.getRawParameterValue (ParamID::bypass);
}

void CeilProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    const int ch = getTotalNumOutputChannels();
    limiter.prepare (sampleRate, ch, 2.0);
    loudness.prepare (sampleRate, ch);
    setLatencySamples (limiter.getLatencySamples());

    bypassMix.reset (sampleRate, 0.01);
    dryBuffer.setSize (ch, samplesPerBlock);
}

bool CeilProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& out = layouts.getMainOutputChannelSet();
    if (out != juce::AudioChannelSet::mono() && out != juce::AudioChannelSet::stereo())
        return false;
    return layouts.getMainInputChannelSet() == out;
}

void CeilProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const auto totalIn  = getTotalNumInputChannels();
    const auto totalOut = getTotalNumOutputChannels();
    for (int ch = totalIn; ch < totalOut; ++ch)
        buffer.clear (ch, 0, buffer.getNumSamples());

    limiter.setParameters (pCeiling->load(), pRelease->load(), pGain->load());

    meters.pushInputPeak (buffer.getMagnitude (0, buffer.getNumSamples()));

    const bool bypassed = *pBypass > 0.5f;
    bypassMix.setTargetValue (bypassed ? 1.0f : 0.0f);
    dryBuffer.makeCopyOf (buffer, true);

    limiter.process (buffer.getArrayOfWritePointers(), totalOut, buffer.getNumSamples());

    if (bypassMix.isSmoothing() || bypassed)
    {
        const auto n = buffer.getNumSamples();
        for (int s = 0; s < n; ++s)
        {
            const auto dryAmt = bypassMix.getNextValue();
            const auto wetAmt = 1.0f - dryAmt;
            for (int ch = 0; ch < totalOut; ++ch)
            {
                auto* w = buffer.getWritePointer (ch);
                const auto* d = dryBuffer.getReadPointer (ch);
                w[s] = w[s] * wetAmt + d[s] * dryAmt;
            }
        }
    }

    // Loudness of the output.
    const auto n = buffer.getNumSamples();
    for (int s = 0; s < n; ++s)
    {
        const float* fr[2] = { &buffer.getReadPointer (0)[s],
                               &buffer.getReadPointer (totalOut > 1 ? 1 : 0)[s] };
        loudness.push (fr, (size_t) totalOut);
    }
    momLufs.store (loudness.momentaryLufs());
    stLufs.store  (loudness.shortTermLufs());

    meters.pushOutputPeak (buffer.getMagnitude (0, buffer.getNumSamples()));
    meters.pushGainReduction (bypassed ? 0.0f : limiter.getGainReductionDb());
}

juce::AudioProcessorEditor* CeilProcessor::createEditor() { return new CeilEditor (*this); }

int CeilProcessor::getNumPrograms() { return (int) Presets::getFactoryPresets().size(); }

void CeilProcessor::setCurrentProgram (int index)
{
    if (index < 0 || index >= getNumPrograms()) return;
    currentProgram = index;
    Presets::apply (index, apvts);
}

const juce::String CeilProcessor::getProgramName (int index)
{
    const auto& p = Presets::getFactoryPresets();
    return (index >= 0 && index < (int) p.size()) ? p[(size_t) index].name : juce::String{};
}

void CeilProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    if (auto state = apvts.copyState(); state.isValid())
    {
        juce::MemoryOutputStream mos (dest, false);
        state.writeToStream (mos);
    }
}

void CeilProcessor::setStateInformation (const void* data, int size)
{
    auto tree = juce::ValueTree::readFromData (data, (size_t) size);
    if (tree.isValid()) apvts.replaceState (tree);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new CeilProcessor(); }
