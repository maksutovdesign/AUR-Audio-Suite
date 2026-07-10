#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Presets.h"

ImagerProcessor::ImagerProcessor()
    : AudioProcessor (BusesProperties()
        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMETERS", Params::createLayout())
{
    pWidth  = apvts.getRawParameterValue (ParamID::width);
    pMono   = apvts.getRawParameterValue (ParamID::monoBelow);
    pBal    = apvts.getRawParameterValue (ParamID::balance);
    pBypass = apvts.getRawParameterValue (ParamID::bypass);
}

void ImagerProcessor::prepareToPlay (double sampleRate, int)
{
    imager.prepare (sampleRate, getTotalNumOutputChannels());
}

bool ImagerProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& out = layouts.getMainOutputChannelSet();
    if (out != juce::AudioChannelSet::mono() && out != juce::AudioChannelSet::stereo())
        return false;
    return layouts.getMainInputChannelSet() == out;
}

void ImagerProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const auto totalOut = getTotalNumOutputChannels();
    const int n = buffer.getNumSamples();

    meters.pushInputPeak (buffer.getMagnitude (0, n));

    if (*pBypass < 0.5f && totalOut > 1)
    {
        imager.setParameters (pWidth->load(), pMono->load(), pBal->load());
        imager.process (buffer.getWritePointer (0), buffer.getWritePointer (1), n);
    }

    // Correlation of the output.
    if (totalOut > 1)
    {
        const float* l = buffer.getReadPointer (0);
        const float* r = buffer.getReadPointer (1);
        double sLR = 0, sLL = 0, sRR = 0;
        for (int s = 0; s < n; ++s) { sLR += (double) l[s]*r[s]; sLL += (double) l[s]*l[s]; sRR += (double) r[s]*r[s]; }
        const double d = std::sqrt (sLL * sRR);
        meters.pushCorrelation (d > 1e-9 ? (float) (sLR / d) : 0.0f);
    }

    meters.pushOutputPeak (buffer.getMagnitude (0, n));
}

juce::AudioProcessorEditor* ImagerProcessor::createEditor() { return new ImagerEditor (*this); }

int ImagerProcessor::getNumPrograms() { return (int) Presets::getFactoryPresets().size(); }

void ImagerProcessor::setCurrentProgram (int index)
{
    if (index < 0 || index >= getNumPrograms()) return;
    currentProgram = index;
    Presets::apply (index, apvts);
}

const juce::String ImagerProcessor::getProgramName (int index)
{
    const auto& p = Presets::getFactoryPresets();
    return (index >= 0 && index < (int) p.size()) ? p[(size_t) index].name : juce::String{};
}

void ImagerProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    if (auto state = apvts.copyState(); state.isValid())
    {
        juce::MemoryOutputStream mos (dest, false);
        state.writeToStream (mos);
    }
}

void ImagerProcessor::setStateInformation (const void* data, int size)
{
    auto tree = juce::ValueTree::readFromData (data, (size_t) size);
    if (tree.isValid()) apvts.replaceState (tree);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new ImagerProcessor(); }
