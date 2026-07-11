#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Presets.h"

GateProcessor::GateProcessor()
    : AudioProcessor (BusesProperties()
        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMETERS", Params::createLayout())
{
    pThr   = apvts.getRawParameterValue (ParamID::threshold);
    pRange = apvts.getRawParameterValue (ParamID::range);
    pAtk   = apvts.getRawParameterValue (ParamID::attack);
    pHold  = apvts.getRawParameterValue (ParamID::hold);
    pRel   = apvts.getRawParameterValue (ParamID::release);
    pBypass = apvts.getRawParameterValue (ParamID::bypass);
}

void GateProcessor::prepareToPlay (double sampleRate, int)
{
    gate.prepare (sampleRate, getTotalNumOutputChannels());
}

bool GateProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& out = layouts.getMainOutputChannelSet();
    if (out != juce::AudioChannelSet::mono() && out != juce::AudioChannelSet::stereo())
        return false;
    return layouts.getMainInputChannelSet() == out;
}

void GateProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const auto totalIn  = getTotalNumInputChannels();
    const auto totalOut = getTotalNumOutputChannels();
    for (int ch = totalIn; ch < totalOut; ++ch)
        buffer.clear (ch, 0, buffer.getNumSamples());

    const int n = buffer.getNumSamples();
    meters.pushInputPeak (buffer.getMagnitude (0, n));

    if (*pBypass < 0.5f)
    {
        gate.setParameters (pThr->load(), pRange->load(), pAtk->load(), pHold->load(), pRel->load());
        gate.process (buffer.getArrayOfWritePointers(), totalOut, n);
    }

    meters.pushOutputPeak (buffer.getMagnitude (0, n));
    meters.pushGainReduction (*pBypass > 0.5f ? 0.0f : gate.getGainReductionDb());
}

juce::AudioProcessorEditor* GateProcessor::createEditor() { return new GateEditor (*this); }

int GateProcessor::getNumPrograms() { return (int) Presets::getFactoryPresets().size(); }

void GateProcessor::setCurrentProgram (int index)
{
    if (index < 0 || index >= getNumPrograms()) return;
    currentProgram = index;
    Presets::apply (index, apvts);
}

const juce::String GateProcessor::getProgramName (int index)
{
    const auto& p = Presets::getFactoryPresets();
    return (index >= 0 && index < (int) p.size()) ? p[(size_t) index].name : juce::String{};
}

void GateProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    if (auto state = apvts.copyState(); state.isValid())
    {
        juce::MemoryOutputStream mos (dest, false);
        state.writeToStream (mos);
    }
}

void GateProcessor::setStateInformation (const void* data, int size)
{
    auto tree = juce::ValueTree::readFromData (data, (size_t) size);
    if (tree.isValid()) apvts.replaceState (tree);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new GateProcessor(); }
