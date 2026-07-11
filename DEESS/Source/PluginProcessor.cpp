#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Presets.h"

DeessProcessor::DeessProcessor()
    : AudioProcessor (BusesProperties()
        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMETERS", Params::createLayout())
{
    pFreq   = apvts.getRawParameterValue (ParamID::freq);
    pThr    = apvts.getRawParameterValue (ParamID::threshold);
    pRange  = apvts.getRawParameterValue (ParamID::range);
    pListen = apvts.getRawParameterValue (ParamID::listen);
    pBypass = apvts.getRawParameterValue (ParamID::bypass);
}

void DeessProcessor::prepareToPlay (double sampleRate, int)
{
    deesser.prepare (sampleRate, getTotalNumOutputChannels());
}

bool DeessProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& out = layouts.getMainOutputChannelSet();
    if (out != juce::AudioChannelSet::mono() && out != juce::AudioChannelSet::stereo())
        return false;
    return layouts.getMainInputChannelSet() == out;
}

void DeessProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
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
        deesser.setParameters (pFreq->load(), pThr->load(), pRange->load());
        deesser.process (buffer.getArrayOfWritePointers(), totalOut, n, *pListen > 0.5f);
    }

    meters.pushOutputPeak (buffer.getMagnitude (0, n));
    meters.pushGainReduction (*pBypass > 0.5f ? 0.0f : deesser.getReductionDb());
}

juce::AudioProcessorEditor* DeessProcessor::createEditor() { return new DeessEditor (*this); }

int DeessProcessor::getNumPrograms() { return (int) Presets::getFactoryPresets().size(); }

void DeessProcessor::setCurrentProgram (int index)
{
    if (index < 0 || index >= getNumPrograms()) return;
    currentProgram = index;
    Presets::apply (index, apvts);
}

const juce::String DeessProcessor::getProgramName (int index)
{
    const auto& p = Presets::getFactoryPresets();
    return (index >= 0 && index < (int) p.size()) ? p[(size_t) index].name : juce::String{};
}

void DeessProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    if (auto state = apvts.copyState(); state.isValid())
    {
        juce::MemoryOutputStream mos (dest, false);
        state.writeToStream (mos);
    }
}

void DeessProcessor::setStateInformation (const void* data, int size)
{
    auto tree = juce::ValueTree::readFromData (data, (size_t) size);
    if (tree.isValid()) apvts.replaceState (tree);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new DeessProcessor(); }
