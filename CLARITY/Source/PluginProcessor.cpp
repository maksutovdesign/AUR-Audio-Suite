#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Presets.h"

ClarityProcessor::ClarityProcessor()
    : AudioProcessor (BusesProperties()
        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMETERS", Params::createLayout())
{
    pDepth  = apvts.getRawParameterValue (ParamID::depth);
    pSens   = apvts.getRawParameterValue (ParamID::sens);
    pSharp  = apvts.getRawParameterValue (ParamID::sharpness);
    pMix    = apvts.getRawParameterValue (ParamID::mix);
    pDelta  = apvts.getRawParameterValue (ParamID::delta);
    pBypass = apvts.getRawParameterValue (ParamID::bypass);
}

void ClarityProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    const int ch = getTotalNumOutputChannels();
    suppressor.prepare (sampleRate, samplesPerBlock, ch);
    bypassMix.reset (sampleRate, 0.01);
    dryBuffer.setSize (ch, samplesPerBlock);
}

bool ClarityProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& out = layouts.getMainOutputChannelSet();
    if (out != juce::AudioChannelSet::mono() && out != juce::AudioChannelSet::stereo())
        return false;
    return layouts.getMainInputChannelSet() == out;
}

void ClarityProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const auto totalIn  = getTotalNumInputChannels();
    const auto totalOut = getTotalNumOutputChannels();
    for (int ch = totalIn; ch < totalOut; ++ch)
        buffer.clear (ch, 0, buffer.getNumSamples());

    // Map sensitivity 0..100 → threshold +8 dB (gentle) .. -2 dB (aggressive).
    const float threshold = 8.0f - (pSens->load() / 100.0f) * 10.0f;
    suppressor.setParameters (pDepth->load(), threshold, pSharp->load(), pMix->load());

    meters.pushInputPeak (buffer.getMagnitude (0, buffer.getNumSamples()));

    const bool bypassed = *pBypass > 0.5f;
    const bool delta    = *pDelta > 0.5f;
    bypassMix.setTargetValue (bypassed ? 1.0f : 0.0f);
    dryBuffer.makeCopyOf (buffer, true);

    suppressor.process (buffer.getArrayOfWritePointers(), totalOut, buffer.getNumSamples(), delta);

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

    meters.pushOutputPeak (buffer.getMagnitude (0, buffer.getNumSamples()));
}

juce::AudioProcessorEditor* ClarityProcessor::createEditor() { return new ClarityEditor (*this); }

int ClarityProcessor::getNumPrograms() { return (int) Presets::getFactoryPresets().size(); }

void ClarityProcessor::setCurrentProgram (int index)
{
    if (index < 0 || index >= getNumPrograms()) return;
    currentProgram = index;
    Presets::apply (index, apvts);
}

const juce::String ClarityProcessor::getProgramName (int index)
{
    const auto& p = Presets::getFactoryPresets();
    return (index >= 0 && index < (int) p.size()) ? p[(size_t) index].name : juce::String{};
}

void ClarityProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    if (auto state = apvts.copyState(); state.isValid())
    {
        juce::MemoryOutputStream mos (dest, false);
        state.writeToStream (mos);
    }
}

void ClarityProcessor::setStateInformation (const void* data, int size)
{
    auto tree = juce::ValueTree::readFromData (data, (size_t) size);
    if (tree.isValid()) apvts.replaceState (tree);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new ClarityProcessor(); }
