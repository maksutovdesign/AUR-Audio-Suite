#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Presets.h"

DelayProcessor::DelayProcessor()
    : AudioProcessor (BusesProperties()
        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMETERS", Params::createLayout())
{
    pTime  = apvts.getRawParameterValue (ParamID::time);
    pFb    = apvts.getRawParameterValue (ParamID::feedback);
    pDamp  = apvts.getRawParameterValue (ParamID::damp);
    pWidth = apvts.getRawParameterValue (ParamID::width);
    pPing  = apvts.getRawParameterValue (ParamID::pingpong);
    pMix   = apvts.getRawParameterValue (ParamID::mix);
    pBypass = apvts.getRawParameterValue (ParamID::bypass);
}

void DelayProcessor::prepareToPlay (double sampleRate, int)
{
    delay.prepare (sampleRate, 2.0);
}

bool DelayProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& out = layouts.getMainOutputChannelSet();
    if (out != juce::AudioChannelSet::mono() && out != juce::AudioChannelSet::stereo())
        return false;
    return layouts.getMainInputChannelSet() == out;
}

void DelayProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const auto totalOut = getTotalNumOutputChannels();
    const int n = buffer.getNumSamples();

    meters.pushInputPeak (buffer.getMagnitude (0, n));

    if (*pBypass < 0.5f && totalOut > 1)
    {
        delay.setParameters (pTime->load(), pFb->load(), pDamp->load(),
                             pMix->load(), *pPing > 0.5f, pWidth->load());
        delay.process (buffer.getWritePointer (0), buffer.getWritePointer (1), n);
    }

    meters.pushOutputPeak (buffer.getMagnitude (0, n));
}

juce::AudioProcessorEditor* DelayProcessor::createEditor() { return new DelayEditor (*this); }

int DelayProcessor::getNumPrograms() { return (int) Presets::getFactoryPresets().size(); }

void DelayProcessor::setCurrentProgram (int index)
{
    if (index < 0 || index >= getNumPrograms()) return;
    currentProgram = index;
    Presets::apply (index, apvts);
}

const juce::String DelayProcessor::getProgramName (int index)
{
    const auto& p = Presets::getFactoryPresets();
    return (index >= 0 && index < (int) p.size()) ? p[(size_t) index].name : juce::String{};
}

void DelayProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    if (auto state = apvts.copyState(); state.isValid())
    {
        juce::MemoryOutputStream mos (dest, false);
        state.writeToStream (mos);
    }
}

void DelayProcessor::setStateInformation (const void* data, int size)
{
    auto tree = juce::ValueTree::readFromData (data, (size_t) size);
    if (tree.isValid()) apvts.replaceState (tree);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new DelayProcessor(); }
