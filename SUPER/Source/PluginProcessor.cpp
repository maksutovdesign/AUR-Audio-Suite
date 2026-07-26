#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Presets.h"

SuperProcessor::SuperProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMETERS", Params::createLayout())
{
    params.bind (apvts);
    pArpOn=apvts.getRawParameterValue(ParamID::arpon); pArpMode=apvts.getRawParameterValue(ParamID::arpmode);
    pArpRate=apvts.getRawParameterValue(ParamID::arprate); pArpOct=apvts.getRawParameterValue(ParamID::arpoct); pArpGate=apvts.getRawParameterValue(ParamID::arpgate);
    pDrive  = apvts.getRawParameterValue (ParamID::drive);
    pVolume = apvts.getRawParameterValue (ParamID::volume);
    synth.addSound (new SuperSound());
    for (int i = 0; i < 8; ++i) synth.addVoice (new SuperVoice (params));
    synth.setNoteStealingEnabled (true);
}

void SuperProcessor::prepareToPlay (double sr, int)
{
    synth.setCurrentPlaybackSampleRate (sr);
    for (int i = 0; i < synth.getNumVoices(); ++i)
        if (auto* v = dynamic_cast<SuperVoice*> (synth.getVoice (i))) v->prepareVoice (sr);
    drive.prepare (sr); drive.setFlavor (aur::ADAASaturator::Flavor::Tube);
    volGain.reset (sr, 0.02);
}

bool SuperProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& o = layouts.getMainOutputChannelSet();
    return o == juce::AudioChannelSet::mono() || o == juce::AudioChannelSet::stereo();
}

void SuperProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    const int totalOut = getTotalNumOutputChannels();
    const int n = buffer.getNumSamples();
    buffer.clear();
    keyboardState.processNextMidiBuffer (midi, 0, n, true);
    {
        double bpm=120.0, ppq=0.0; bool playing=false;
        if (auto* ph=getPlayHead())
            if (auto pos=ph->getPosition(); pos.hasValue())
            {
                if (auto b=pos->getBpm()) bpm=*b;
                if (auto q=pos->getPpqPosition()) ppq=*q;
                playing=pos->getIsPlaying();
            }
        arp.setParameters (*pArpOn>0.5f,(int)*pArpMode,(int)*pArpRate,(int)*pArpOct,*pArpGate);
        arp.process (midi, bpm, playing, ppq, n);
    }
    synth.renderNextBlock (buffer, midi, 0, n);

    juce::dsp::AudioBlock<float> block (buffer);
    drive.setParameters (*pDrive * 100.0f, 100.0f);
    drive.process (block);

    volGain.setTargetValue (juce::Decibels::decibelsToGain (pVolume->load()));
    for (int s = 0; s < n; ++s) { const float g = volGain.getNextValue(); for (int ch = 0; ch < totalOut; ++ch) buffer.getWritePointer (ch)[s] *= g; }
    meters.pushOutputPeak (buffer.getMagnitude (0, n));
}

juce::AudioProcessorEditor* SuperProcessor::createEditor() { return new SuperEditor (*this); }
int SuperProcessor::getNumPrograms() { return (int) Presets::getFactoryPresets().size(); }
void SuperProcessor::setCurrentProgram (int i) { if (i < 0 || i >= getNumPrograms()) return; currentProgram = i; Presets::apply (i, apvts); }
const juce::String SuperProcessor::getProgramName (int i) { const auto& p = Presets::getFactoryPresets(); return (i >= 0 && i < (int) p.size()) ? p[(size_t) i].name : juce::String{}; }
void SuperProcessor::getStateInformation (juce::MemoryBlock& d) { if (auto s = apvts.copyState(); s.isValid()) { juce::MemoryOutputStream mos (d, false); s.writeToStream (mos); } }
void SuperProcessor::setStateInformation (const void* d, int s) { auto t = juce::ValueTree::readFromData (d, (size_t) s); if (t.isValid()) apvts.replaceState (t); }

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new SuperProcessor(); }
