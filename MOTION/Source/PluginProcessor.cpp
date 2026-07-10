#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Presets.h"

MotionProcessor::MotionProcessor()
    : AudioProcessor (BusesProperties()
        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMETERS", Params::createLayout())
{
    pf[0] = apvts.getRawParameterValue (ParamID::f1); pt[0] = apvts.getRawParameterValue (ParamID::t1); pr[0] = apvts.getRawParameterValue (ParamID::r1);
    pf[1] = apvts.getRawParameterValue (ParamID::f2); pt[1] = apvts.getRawParameterValue (ParamID::t2); pr[1] = apvts.getRawParameterValue (ParamID::r2);
    pf[2] = apvts.getRawParameterValue (ParamID::f3); pt[2] = apvts.getRawParameterValue (ParamID::t3); pr[2] = apvts.getRawParameterValue (ParamID::r3);
    pQ = apvts.getRawParameterValue (ParamID::q);
    pBypass = apvts.getRawParameterValue (ParamID::bypass);
}

void MotionProcessor::prepareToPlay (double sampleRate, int)
{
    eq.prepare (sampleRate, getTotalNumOutputChannels());
}

bool MotionProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& out = layouts.getMainOutputChannelSet();
    if (out != juce::AudioChannelSet::mono() && out != juce::AudioChannelSet::stereo())
        return false;
    return layouts.getMainInputChannelSet() == out;
}

void MotionProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const auto totalIn  = getTotalNumInputChannels();
    const auto totalOut = getTotalNumOutputChannels();
    for (int ch = totalIn; ch < totalOut; ++ch)
        buffer.clear (ch, 0, buffer.getNumSamples());

    const float qv = pQ->load();
    for (size_t b = 0; b < 3; ++b)
        eq.setBand (b, pf[b]->load(), pt[b]->load(), pr[b]->load(), qv);

    if (*pBypass < 0.5f)
        eq.process (buffer.getArrayOfWritePointers(), totalOut, buffer.getNumSamples());

    const int n = buffer.getNumSamples();
    const auto* l = buffer.getReadPointer (0);
    const auto* r = totalOut > 1 ? buffer.getReadPointer (1) : l;
    for (int s = 0; s < n; ++s)
        analyzer.push (0.5f * (l[s] + r[s]));
}

juce::AudioProcessorEditor* MotionProcessor::createEditor() { return new MotionEditor (*this); }

int MotionProcessor::getNumPrograms() { return (int) Presets::getFactoryPresets().size(); }

void MotionProcessor::setCurrentProgram (int index)
{
    if (index < 0 || index >= getNumPrograms()) return;
    currentProgram = index;
    Presets::apply (index, apvts);
}

const juce::String MotionProcessor::getProgramName (int index)
{
    const auto& p = Presets::getFactoryPresets();
    return (index >= 0 && index < (int) p.size()) ? p[(size_t) index].name : juce::String{};
}

void MotionProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    if (auto state = apvts.copyState(); state.isValid())
    {
        juce::MemoryOutputStream mos (dest, false);
        state.writeToStream (mos);
    }
}

void MotionProcessor::setStateInformation (const void* data, int size)
{
    auto tree = juce::ValueTree::readFromData (data, (size_t) size);
    if (tree.isValid()) apvts.replaceState (tree);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new MotionProcessor(); }
