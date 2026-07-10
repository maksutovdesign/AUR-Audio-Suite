#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Presets.h"

HazeProcessor::HazeProcessor()
    : AudioProcessor (BusesProperties()
        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMETERS", Params::createLayout())
{
    pSize   = apvts.getRawParameterValue (ParamID::size);
    pDecay  = apvts.getRawParameterValue (ParamID::decay);
    pDamp   = apvts.getRawParameterValue (ParamID::damp);
    pPre    = apvts.getRawParameterValue (ParamID::predelay);
    pWidth  = apvts.getRawParameterValue (ParamID::width);
    pMix    = apvts.getRawParameterValue (ParamID::mix);
    pBypass = apvts.getRawParameterValue (ParamID::bypass);
}

void HazeProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    reverb.prepare (sampleRate, getTotalNumOutputChannels());
    wet.setSize (2, samplesPerBlock);
}

bool HazeProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& out = layouts.getMainOutputChannelSet();
    if (out != juce::AudioChannelSet::mono() && out != juce::AudioChannelSet::stereo())
        return false;
    return layouts.getMainInputChannelSet() == out;
}

void HazeProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const auto totalIn  = getTotalNumInputChannels();
    const auto totalOut = getTotalNumOutputChannels();
    for (int ch = totalIn; ch < totalOut; ++ch)
        buffer.clear (ch, 0, buffer.getNumSamples());

    const int n = buffer.getNumSamples();
    meters.pushInputPeak (buffer.getMagnitude (0, n));

    reverb.setParameters (pSize->load() / 100.0f, pDecay->load(),
                          pDamp->load() / 100.0f, pPre->load(), pWidth->load());

    if (*pBypass > 0.5f)
    {
        meters.pushOutputPeak (buffer.getMagnitude (0, n));
        return;
    }

    // Build a stereo wet copy of the input, reverberate it.
    wet.setSize (2, n, false, false, true);
    wet.copyFrom (0, 0, buffer, 0, 0, n);
    wet.copyFrom (1, 0, buffer, totalOut > 1 ? 1 : 0, 0, n);
    reverb.process (wet.getWritePointer (0), wet.getWritePointer (1), n);

    const float mix   = pMix->load() / 100.0f;
    const float width = pWidth->load() / 100.0f;

    for (int s = 0; s < n; ++s)
    {
        float wl = wet.getSample (0, s);
        float wr = wet.getSample (1, s);
        // Width via mid/side on the wet signal.
        const float mid  = 0.5f * (wl + wr);
        const float side = 0.5f * (wl - wr) * width;
        wl = mid + side;
        wr = mid - side;

        for (int ch = 0; ch < totalOut; ++ch)
        {
            auto* d = buffer.getWritePointer (ch);
            const float w = (ch == 0) ? wl : wr;
            d[s] = d[s] * (1.0f - mix) + w * mix;
        }
    }

    meters.pushOutputPeak (buffer.getMagnitude (0, n));
}

juce::AudioProcessorEditor* HazeProcessor::createEditor() { return new HazeEditor (*this); }

int HazeProcessor::getNumPrograms() { return (int) Presets::getFactoryPresets().size(); }

void HazeProcessor::setCurrentProgram (int index)
{
    if (index < 0 || index >= getNumPrograms()) return;
    currentProgram = index;
    Presets::apply (index, apvts);
}

const juce::String HazeProcessor::getProgramName (int index)
{
    const auto& p = Presets::getFactoryPresets();
    return (index >= 0 && index < (int) p.size()) ? p[(size_t) index].name : juce::String{};
}

void HazeProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    if (auto state = apvts.copyState(); state.isValid())
    {
        juce::MemoryOutputStream mos (dest, false);
        state.writeToStream (mos);
    }
}

void HazeProcessor::setStateInformation (const void* data, int size)
{
    auto tree = juce::ValueTree::readFromData (data, (size_t) size);
    if (tree.isValid()) apvts.replaceState (tree);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new HazeProcessor(); }
