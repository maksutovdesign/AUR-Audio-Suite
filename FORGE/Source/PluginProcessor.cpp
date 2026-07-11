#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Presets.h"

ForgeProcessor::ForgeProcessor()
    : AudioProcessor (BusesProperties()
        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMETERS", Params::createLayout())
{
    pChar   = apvts.getRawParameterValue (ParamID::character);
    pFlavor = apvts.getRawParameterValue (ParamID::flavor);
    pInput  = apvts.getRawParameterValue (ParamID::input);
    pHpf    = apvts.getRawParameterValue (ParamID::hpf);
    pTone   = apvts.getRawParameterValue (ParamID::tone);
    pOutput = apvts.getRawParameterValue (ParamID::output);
    pBypass = apvts.getRawParameterValue (ParamID::bypass);
}

void ForgeProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    fs = sampleRate;
    juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) samplesPerBlock, (juce::uint32) getTotalNumOutputChannels() };

    inGain.prepare (sampleRate);
    outGain.prepare (sampleRate);
    sat.prepare (sampleRate);
    comp.prepare (spec);
    tone.prepare (spec);
    for (auto& f : hpf) f.setHighpass (sampleRate, 20.0, 0.707);

    bypassMix.reset (sampleRate, 0.01);
    dryBuffer.setSize (getTotalNumOutputChannels(), samplesPerBlock);
}

bool ForgeProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& out = layouts.getMainOutputChannelSet();
    if (out != juce::AudioChannelSet::mono() && out != juce::AudioChannelSet::stereo())
        return false;
    return layouts.getMainInputChannelSet() == out;
}

void ForgeProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const auto totalIn  = getTotalNumInputChannels();
    const auto totalOut = getTotalNumOutputChannels();
    for (int ch = totalIn; ch < totalOut; ++ch)
        buffer.clear (ch, 0, buffer.getNumSamples());

    const int n = buffer.getNumSamples();
    const float ch01 = pChar->load() / 100.0f;   // 0 = clean, 1 = molten
    const bool hpOn = *pHpf > 21.0f;

    // --- CHARACTER macro maps one knob onto the whole chain ---
    inGain.setGainDecibels  (pInput->load());
    outGain.setGainDecibels (pOutput->load());
    if (hpOn) for (auto& f : hpf) f.setHighpass (fs, pHpf->load(), 0.707);

    sat.setFlavor (static_cast<aur::ADAASaturator::Flavor> ((int) (*pFlavor + 0.5f)));
    sat.setParameters (ch01 * 100.0f, 100.0f);                       // drive rises with character

    comp.setParameters (-8.0f - ch01 * 20.0f,                        // lower threshold
                        1.3f + ch01 * 3.5f,                          // higher ratio
                        15.0f, 160.0f, ch01 * 4.0f, 100.0f);         // more makeup

    tone.setTone (pTone->load());

    meters.pushInputPeak (buffer.getMagnitude (0, n));

    const bool bypassed = *pBypass > 0.5f;
    bypassMix.setTargetValue (bypassed ? 1.0f : 0.0f);
    dryBuffer.makeCopyOf (buffer, true);

    juce::dsp::AudioBlock<float> block (buffer);
    inGain.process (block);

    if (hpOn)
        for (int c = 0; c < totalOut; ++c)
        {
            auto* d = buffer.getWritePointer (c);
            auto& f = hpf[(size_t) juce::jmin (c, 1)];
            for (int s = 0; s < n; ++s) d[s] = f.process (d[s]);
        }

    sat.process (block);
    comp.process (block);
    tone.process (block);
    outGain.process (block);

    // Gentle safety ceiling (~ -0.3 dBFS) so the strip never hard-clips.
    const float ceil = 0.97f;
    for (int c = 0; c < totalOut; ++c)
    {
        auto* d = buffer.getWritePointer (c);
        for (int s = 0; s < n; ++s)
            d[s] = juce::jlimit (-ceil, ceil, d[s]);
    }

    // Click-free bypass crossfade.
    if (bypassMix.isSmoothing() || bypassed)
    {
        for (int s = 0; s < n; ++s)
        {
            const auto dry = bypassMix.getNextValue();
            const auto wet = 1.0f - dry;
            for (int c = 0; c < totalOut; ++c)
            {
                auto* w = buffer.getWritePointer (c);
                const auto* dd = dryBuffer.getReadPointer (c);
                w[s] = w[s] * wet + dd[s] * dry;
            }
        }
    }

    meters.pushOutputPeak (buffer.getMagnitude (0, n));
    meters.pushGainReduction (bypassed ? 0.0f : comp.getGainReductionDb());

    const auto* l = buffer.getReadPointer (0);
    const auto* r = totalOut > 1 ? buffer.getReadPointer (1) : l;
    for (int s = 0; s < n; ++s) analyzer.push (0.5f * (l[s] + r[s]));
}

juce::AudioProcessorEditor* ForgeProcessor::createEditor() { return new ForgeEditor (*this); }

int ForgeProcessor::getNumPrograms() { return (int) Presets::getFactoryPresets().size(); }

void ForgeProcessor::setCurrentProgram (int index)
{
    if (index < 0 || index >= getNumPrograms()) return;
    currentProgram = index;
    Presets::apply (index, apvts);
}

const juce::String ForgeProcessor::getProgramName (int index)
{
    const auto& p = Presets::getFactoryPresets();
    return (index >= 0 && index < (int) p.size()) ? p[(size_t) index].name : juce::String{};
}

void ForgeProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    if (auto state = apvts.copyState(); state.isValid())
    {
        juce::MemoryOutputStream mos (dest, false);
        state.writeToStream (mos);
    }
}

void ForgeProcessor::setStateInformation (const void* data, int size)
{
    auto tree = juce::ValueTree::readFromData (data, (size_t) size);
    if (tree.isValid()) apvts.replaceState (tree);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new ForgeProcessor(); }
