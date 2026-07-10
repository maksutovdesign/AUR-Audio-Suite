#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Presets.h"

PrismProcessor::PrismProcessor()
    : AudioProcessor (BusesProperties()
        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMETERS", Params::createLayout())
{
    const char* ids[9] = { ParamID::hpFreq, ParamID::lsFreq, ParamID::lsGain,
                           ParamID::bellFreq, ParamID::bellGain, ParamID::bellQ,
                           ParamID::hsFreq, ParamID::hsGain, ParamID::lpFreq };
    for (int i = 0; i < 9; ++i) p[i] = apvts.getRawParameterValue (ids[i]);
    pBypass = apvts.getRawParameterValue (ParamID::bypass);
}

void PrismProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    sr = sampleRate;
    eq.prepare (sampleRate, getTotalNumOutputChannels());
    bypassMix.reset (sampleRate, 0.01);
    dryBuffer.setSize (getTotalNumOutputChannels(), samplesPerBlock);
}

bool PrismProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& out = layouts.getMainOutputChannelSet();
    if (out != juce::AudioChannelSet::mono() && out != juce::AudioChannelSet::stereo())
        return false;
    return layouts.getMainInputChannelSet() == out;
}

void PrismProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const auto totalIn  = getTotalNumInputChannels();
    const auto totalOut = getTotalNumOutputChannels();
    for (int ch = totalIn; ch < totalOut; ++ch)
        buffer.clear (ch, 0, buffer.getNumSamples());

    eq.setParameters (p[0]->load(), p[1]->load(), p[2]->load(),
                      p[3]->load(), p[4]->load(), p[5]->load(),
                      p[6]->load(), p[7]->load(), p[8]->load());

    const bool bypassed = *pBypass > 0.5f;
    bypassMix.setTargetValue (bypassed ? 1.0f : 0.0f);
    dryBuffer.makeCopyOf (buffer, true);

    eq.process (buffer.getArrayOfWritePointers(), totalOut, buffer.getNumSamples());

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

    const auto n = buffer.getNumSamples();
    const auto* l = buffer.getReadPointer (0);
    const auto* r = totalOut > 1 ? buffer.getReadPointer (1) : l;
    for (int s = 0; s < n; ++s)
        analyzer.push (0.5f * (l[s] + r[s]));
}

juce::AudioProcessorEditor* PrismProcessor::createEditor() { return new PrismEditor (*this); }

int PrismProcessor::getNumPrograms() { return (int) Presets::getFactoryPresets().size(); }

void PrismProcessor::setCurrentProgram (int index)
{
    if (index < 0 || index >= getNumPrograms()) return;
    currentProgram = index;
    Presets::apply (index, apvts);
}

const juce::String PrismProcessor::getProgramName (int index)
{
    const auto& pr = Presets::getFactoryPresets();
    return (index >= 0 && index < (int) pr.size()) ? pr[(size_t) index].name : juce::String{};
}

void PrismProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    if (auto state = apvts.copyState(); state.isValid())
    {
        juce::MemoryOutputStream mos (dest, false);
        state.writeToStream (mos);
    }
}

void PrismProcessor::setStateInformation (const void* data, int size)
{
    auto tree = juce::ValueTree::readFromData (data, (size_t) size);
    if (tree.isValid()) apvts.replaceState (tree);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new PrismProcessor(); }
