#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Presets.h"

GripProcessor::GripProcessor()
    : AudioProcessor (BusesProperties()
        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMETERS", Params::createLayout())
{
    pThresh  = apvts.getRawParameterValue (ParamID::threshold);
    pRatio   = apvts.getRawParameterValue (ParamID::ratio);
    pAttack  = apvts.getRawParameterValue (ParamID::attack);
    pRelease = apvts.getRawParameterValue (ParamID::release);
    pMakeup  = apvts.getRawParameterValue (ParamID::makeup);
    pMix     = apvts.getRawParameterValue (ParamID::mix);
    pBypass  = apvts.getRawParameterValue (ParamID::bypass);
}

void GripProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate       = sampleRate;
    spec.maximumBlockSize = (juce::uint32) samplesPerBlock;
    spec.numChannels      = (juce::uint32) getTotalNumOutputChannels();

    comp.prepare (spec);
    bypassMix.reset (sampleRate, 0.01);
    dryBuffer.setSize ((int) spec.numChannels, samplesPerBlock);
}

bool GripProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& out = layouts.getMainOutputChannelSet();
    if (out != juce::AudioChannelSet::mono() && out != juce::AudioChannelSet::stereo())
        return false;
    return layouts.getMainInputChannelSet() == out;
}

void GripProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const auto totalIn  = getTotalNumInputChannels();
    const auto totalOut = getTotalNumOutputChannels();
    for (int ch = totalIn; ch < totalOut; ++ch)
        buffer.clear (ch, 0, buffer.getNumSamples());

    comp.setParameters (pThresh->load(), pRatio->load(), pAttack->load(),
                        pRelease->load(), pMakeup->load(), pMix->load());

    meters.pushInputPeak (buffer.getMagnitude (0, buffer.getNumSamples()));

    const bool bypassed = *pBypass > 0.5f;
    bypassMix.setTargetValue (bypassed ? 1.0f : 0.0f);
    dryBuffer.makeCopyOf (buffer, true);

    juce::dsp::AudioBlock<float> block (buffer);
    comp.process (block);

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
    meters.pushGainReduction (bypassed ? 0.0f : comp.getGainReductionDb());
}

juce::AudioProcessorEditor* GripProcessor::createEditor() { return new GripEditor (*this); }

int GripProcessor::getNumPrograms() { return (int) Presets::getFactoryPresets().size(); }

void GripProcessor::setCurrentProgram (int index)
{
    if (index < 0 || index >= getNumPrograms()) return;
    currentProgram = index;
    Presets::apply (index, apvts);
}

const juce::String GripProcessor::getProgramName (int index)
{
    const auto& p = Presets::getFactoryPresets();
    return (index >= 0 && index < (int) p.size()) ? p[(size_t) index].name : juce::String{};
}

void GripProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    if (auto state = apvts.copyState(); state.isValid())
    {
        juce::MemoryOutputStream mos (dest, false);
        state.writeToStream (mos);
    }
}

void GripProcessor::setStateInformation (const void* data, int size)
{
    auto tree = juce::ValueTree::readFromData (data, (size_t) size);
    if (tree.isValid()) apvts.replaceState (tree);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new GripProcessor(); }
