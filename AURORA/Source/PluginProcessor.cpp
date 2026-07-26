#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Presets.h"

AuroraProcessor::AuroraProcessor()
    : AudioProcessor (BusesProperties()
        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMETERS", Params::createLayout())
{
    params.bind (apvts);
    pArpOn=apvts.getRawParameterValue(ParamID::arpon); pArpMode=apvts.getRawParameterValue(ParamID::arpmode);
    pArpRate=apvts.getRawParameterValue(ParamID::arprate); pArpOct=apvts.getRawParameterValue(ParamID::arpoct); pArpGate=apvts.getRawParameterValue(ParamID::arpgate);
    pDrive  = apvts.getRawParameterValue (ParamID::drive);
    pVolume = apvts.getRawParameterValue (ParamID::volume);

    synth.addSound (new AuroraSound());
    for (int i = 0; i < kNumVoices; ++i)
        synth.addVoice (new AuroraVoice (params));
    synth.setNoteStealingEnabled (true);
}

void AuroraProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    synth.setCurrentPlaybackSampleRate (sampleRate);
    arp.prepare (sampleRate);
    for (int i = 0; i < synth.getNumVoices(); ++i)
        if (auto* v = dynamic_cast<AuroraVoice*> (synth.getVoice (i)))
            v->prepareVoice (sampleRate, samplesPerBlock);

    drive.prepare (sampleRate);
    drive.setFlavor (aur::ADAASaturator::Flavor::Tube);
    volGain.reset (sampleRate, 0.02);
}

bool AuroraProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& out = layouts.getMainOutputChannelSet();
    return out == juce::AudioChannelSet::mono() || out == juce::AudioChannelSet::stereo();
}

void AuroraProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    const int totalOut = getTotalNumOutputChannels();
    const int n = buffer.getNumSamples();
    buffer.clear();

    // Merge on-screen keyboard events, then render the synth.
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

    // Global analog-character drive (dosable ADAA warmth) — the AUR signature.
    juce::dsp::AudioBlock<float> block (buffer);
    drive.setParameters (*pDrive * 100.0f, 100.0f);
    drive.process (block);

    // Output volume.
    volGain.setTargetValue (juce::Decibels::decibelsToGain (pVolume->load()));
    for (int s = 0; s < n; ++s)
    {
        const float g = volGain.getNextValue();
        for (int ch = 0; ch < totalOut; ++ch)
            buffer.getWritePointer (ch)[s] *= g;
    }

    meters.pushOutputPeak (buffer.getMagnitude (0, n));
}

juce::AudioProcessorEditor* AuroraProcessor::createEditor() { return new AuroraEditor (*this); }

int AuroraProcessor::getNumPrograms() { return (int) Presets::getFactoryPresets().size(); }

void AuroraProcessor::setCurrentProgram (int index)
{
    if (index < 0 || index >= getNumPrograms()) return;
    currentProgram = index;
    Presets::apply (index, apvts);
}

const juce::String AuroraProcessor::getProgramName (int index)
{
    const auto& p = Presets::getFactoryPresets();
    return (index >= 0 && index < (int) p.size()) ? p[(size_t) index].name : juce::String{};
}

void AuroraProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    if (auto state = apvts.copyState(); state.isValid())
    {
        juce::MemoryOutputStream mos (dest, false);
        state.writeToStream (mos);
    }
}

void AuroraProcessor::setStateInformation (const void* data, int size)
{
    auto tree = juce::ValueTree::readFromData (data, (size_t) size);
    if (tree.isValid()) apvts.replaceState (tree);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new AuroraProcessor(); }
