#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Presets.h"

ChipProcessor::ChipProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMETERS", Params::createLayout())
{
    params.bind (apvts);
    pVolume = apvts.getRawParameterValue (ParamID::volume);
    synth.addSound (new ChipSound());
    for (int i = 0; i < 8; ++i) synth.addVoice (new ChipVoice (params));
    synth.setNoteStealingEnabled (true);
}

void ChipProcessor::prepareToPlay (double sr, int)
{
    synth.setCurrentPlaybackSampleRate (sr);
    for (int i = 0; i < synth.getNumVoices(); ++i)
        if (auto* v = dynamic_cast<ChipVoice*> (synth.getVoice (i))) v->prepareVoice (sr);
    drive.prepare (sr); drive.setFlavor (aur::ADAASaturator::Flavor::Tube);
    volGain.reset (sr, 0.02);
}

bool ChipProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& o = layouts.getMainOutputChannelSet();
    return o == juce::AudioChannelSet::mono() || o == juce::AudioChannelSet::stereo();
}

void ChipProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    const int totalOut = getTotalNumOutputChannels();
    const int n = buffer.getNumSamples();
    buffer.clear();
    keyboardState.processNextMidiBuffer (midi, 0, n, true);
    synth.renderNextBlock (buffer, midi, 0, n);

    volGain.setTargetValue (juce::Decibels::decibelsToGain (pVolume->load()));
    for (int s = 0; s < n; ++s) { const float g = volGain.getNextValue(); for (int ch = 0; ch < totalOut; ++ch) buffer.getWritePointer (ch)[s] *= g; }
    meters.pushOutputPeak (buffer.getMagnitude (0, n));
}

juce::AudioProcessorEditor* ChipProcessor::createEditor() { return new ChipEditor (*this); }
int ChipProcessor::getNumPrograms() { return (int) Presets::getFactoryPresets().size(); }
void ChipProcessor::setCurrentProgram (int i) { if (i < 0 || i >= getNumPrograms()) return; currentProgram = i; Presets::apply (i, apvts); }
const juce::String ChipProcessor::getProgramName (int i) { const auto& p = Presets::getFactoryPresets(); return (i >= 0 && i < (int) p.size()) ? p[(size_t) i].name : juce::String{}; }
void ChipProcessor::getStateInformation (juce::MemoryBlock& d) { if (auto s = apvts.copyState(); s.isValid()) { juce::MemoryOutputStream mos (d, false); s.writeToStream (mos); } }
void ChipProcessor::setStateInformation (const void* d, int s) { auto t = juce::ValueTree::readFromData (d, (size_t) s); if (t.isValid()) apvts.replaceState (t); }

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new ChipProcessor(); }
