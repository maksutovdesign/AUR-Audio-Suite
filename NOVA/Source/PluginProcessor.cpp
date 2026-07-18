#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Presets.h"

NovaProcessor::NovaProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMETERS", Params::createLayout())
{
    params.bind (apvts);
    pDrive  = apvts.getRawParameterValue (ParamID::drive);
    pVolume = apvts.getRawParameterValue (ParamID::volume);

    synth.addSound (new NovaSound());
    for (int i = 0; i < kNumVoices; ++i)
        synth.addVoice (new NovaVoice (params));
    synth.setNoteStealingEnabled (true);
}

void NovaProcessor::prepareToPlay (double sampleRate, int)
{
    synth.setCurrentPlaybackSampleRate (sampleRate);
    for (int i = 0; i < synth.getNumVoices(); ++i)
        if (auto* v = dynamic_cast<NovaVoice*> (synth.getVoice (i)))
            v->prepareVoice (sampleRate);
    drive.prepare (sampleRate);
    drive.setFlavor (aur::ADAASaturator::Flavor::Tube);
    volGain.reset (sampleRate, 0.02);
}

bool NovaProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& out = layouts.getMainOutputChannelSet();
    return out == juce::AudioChannelSet::mono() || out == juce::AudioChannelSet::stereo();
}

void NovaProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    const int totalOut = getTotalNumOutputChannels();
    const int n = buffer.getNumSamples();
    buffer.clear();

    keyboardState.processNextMidiBuffer (midi, 0, n, true);
    synth.renderNextBlock (buffer, midi, 0, n);

    juce::dsp::AudioBlock<float> block (buffer);
    drive.setParameters (*pDrive * 100.0f, 100.0f);
    drive.process (block);

    volGain.setTargetValue (juce::Decibels::decibelsToGain (pVolume->load()));
    for (int s = 0; s < n; ++s)
    {
        const float g = volGain.getNextValue();
        for (int ch = 0; ch < totalOut; ++ch)
            buffer.getWritePointer (ch)[s] *= g;
    }

    meters.pushOutputPeak (buffer.getMagnitude (0, n));
}

juce::AudioProcessorEditor* NovaProcessor::createEditor() { return new NovaEditor (*this); }

int NovaProcessor::getNumPrograms() { return (int) Presets::getFactoryPresets().size(); }

void NovaProcessor::setCurrentProgram (int index)
{
    if (index < 0 || index >= getNumPrograms()) return;
    currentProgram = index;
    Presets::apply (index, apvts);
}

const juce::String NovaProcessor::getProgramName (int index)
{
    const auto& p = Presets::getFactoryPresets();
    return (index >= 0 && index < (int) p.size()) ? p[(size_t) index].name : juce::String{};
}

void NovaProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    if (auto state = apvts.copyState(); state.isValid())
    {
        juce::MemoryOutputStream mos (dest, false);
        state.writeToStream (mos);
    }
}

void NovaProcessor::setStateInformation (const void* data, int size)
{
    auto tree = juce::ValueTree::readFromData (data, (size_t) size);
    if (tree.isValid()) apvts.replaceState (tree);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new NovaProcessor(); }
