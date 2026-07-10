#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Presets.h"

EmberProcessor::EmberProcessor()
    : AudioProcessor (BusesProperties()
        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMETERS", Params::createLayout())
{
    pInput  = apvts.getRawParameterValue (ParamID::inputGain);
    pFlavor = apvts.getRawParameterValue (ParamID::flavor);
    pDrive  = apvts.getRawParameterValue (ParamID::drive);
    pMix    = apvts.getRawParameterValue (ParamID::mix);
    pTone   = apvts.getRawParameterValue (ParamID::tone);
    pOutput = apvts.getRawParameterValue (ParamID::outputGain);
    pBypass = apvts.getRawParameterValue (ParamID::bypass);
}

void EmberProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate       = sampleRate;
    spec.maximumBlockSize = (juce::uint32) samplesPerBlock;
    spec.numChannels      = (juce::uint32) getTotalNumOutputChannels();

    inputGain.prepare (sampleRate);
    outputGain.prepare (sampleRate);
    sat.prepare (sampleRate);
    tone.prepare (spec);

    bypassMix.reset (sampleRate, 0.01);
    dryBuffer.setSize ((int) spec.numChannels, samplesPerBlock);
}

bool EmberProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& out = layouts.getMainOutputChannelSet();
    if (out != juce::AudioChannelSet::mono() && out != juce::AudioChannelSet::stereo())
        return false;
    return layouts.getMainInputChannelSet() == out;
}

void EmberProcessor::updateParams()
{
    inputGain.setGainDecibels  (pInput->load());
    outputGain.setGainDecibels (pOutput->load());

    sat.setFlavor (static_cast<aur::ADAASaturator::Flavor> ((int) (*pFlavor + 0.5f)));
    sat.setParameters (pDrive->load(), pMix->load());

    tone.setTone (pTone->load());
}

void EmberProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const auto totalIn  = getTotalNumInputChannels();
    const auto totalOut = getTotalNumOutputChannels();
    for (int ch = totalIn; ch < totalOut; ++ch)
        buffer.clear (ch, 0, buffer.getNumSamples());

    updateParams();
    meters.pushInputPeak (buffer.getMagnitude (0, buffer.getNumSamples()));

    const bool bypassed = *pBypass > 0.5f;
    bypassMix.setTargetValue (bypassed ? 1.0f : 0.0f);
    dryBuffer.makeCopyOf (buffer, true);

    juce::dsp::AudioBlock<float> block (buffer);
    inputGain.process (block);
    sat.process (block);
    tone.process (block);
    outputGain.process (block);

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

    // Feed the spectrum analyzer with the mono output.
    const auto n = buffer.getNumSamples();
    const auto* l = buffer.getReadPointer (0);
    const auto* r = totalOut > 1 ? buffer.getReadPointer (1) : l;
    for (int s = 0; s < n; ++s)
        analyzer.push (0.5f * (l[s] + r[s]));
}

juce::AudioProcessorEditor* EmberProcessor::createEditor() { return new EmberEditor (*this); }

int EmberProcessor::getNumPrograms() { return (int) Presets::getFactoryPresets().size(); }

void EmberProcessor::setCurrentProgram (int index)
{
    if (index < 0 || index >= getNumPrograms()) return;
    currentProgram = index;
    Presets::apply (index, apvts);
}

const juce::String EmberProcessor::getProgramName (int index)
{
    const auto& p = Presets::getFactoryPresets();
    return (index >= 0 && index < (int) p.size()) ? p[(size_t) index].name : juce::String{};
}

void EmberProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    if (auto state = apvts.copyState(); state.isValid())
    {
        juce::MemoryOutputStream mos (dest, false);
        state.writeToStream (mos);
    }
}

void EmberProcessor::setStateInformation (const void* data, int size)
{
    auto tree = juce::ValueTree::readFromData (data, (size_t) size);
    if (tree.isValid()) apvts.replaceState (tree);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new EmberProcessor(); }
