#include "PluginProcessor.h"
#include "PluginEditor.h"

DenoiseProcessor::DenoiseProcessor()
    : AudioProcessor (BusesProperties()
        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMETERS", Params::createLayout())
{
    pAmount = apvts.getRawParameterValue (ParamID::amount);
    pSens   = apvts.getRawParameterValue (ParamID::sensitivity);
    pBypass = apvts.getRawParameterValue (ParamID::bypass);
}

void DenoiseProcessor::prepareToPlay (double sampleRate, int)
{
    denoiser.prepare (sampleRate, getTotalNumOutputChannels());
    setLatencySamples (aur::SpectralDenoiser::kFFT);
}

bool DenoiseProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& out = layouts.getMainOutputChannelSet();
    if (out != juce::AudioChannelSet::mono() && out != juce::AudioChannelSet::stereo())
        return false;
    return layouts.getMainInputChannelSet() == out;
}

void DenoiseProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const auto totalIn  = getTotalNumInputChannels();
    const auto totalOut = getTotalNumOutputChannels();
    for (int ch = totalIn; ch < totalOut; ++ch)
        buffer.clear (ch, 0, buffer.getNumSamples());

    const int n = buffer.getNumSamples();

    if (*pBypass < 0.5f)
    {
        denoiser.setParameters (pAmount->load(), pSens->load());
        denoiser.process (buffer.getArrayOfWritePointers(), totalOut, n);
    }

    const auto* l = buffer.getReadPointer (0);
    const auto* r = totalOut > 1 ? buffer.getReadPointer (1) : l;
    for (int s = 0; s < n; ++s) analyzer.push (0.5f * (l[s] + r[s]));
}

juce::AudioProcessorEditor* DenoiseProcessor::createEditor() { return new DenoiseEditor (*this); }

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new DenoiseProcessor(); }
