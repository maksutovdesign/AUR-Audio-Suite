#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Presets.h"

PulseProcessor::PulseProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMETERS", Params::createLayout())
{
    pDrive  = apvts.getRawParameterValue (ParamID::drive);
    pVolume = apvts.getRawParameterValue (ParamID::volume);
    for (int v = 0; v < Drum::Count; ++v)
    {
        pLevel[v] = apvts.getRawParameterValue (ParamID::level (v));
        pTune[v]  = apvts.getRawParameterValue (ParamID::tune (v));
        pDecay[v] = apvts.getRawParameterValue (ParamID::decay (v));
    }
}

void PulseProcessor::prepareToPlay (double sr, int)
{
    kick.prepare (sr); snare.prepare (sr); clap.prepare (sr);
    chat.prepare (sr); ohat.prepare (sr); tom.prepare (sr); rim.prepare (sr);
    drive.prepare (sr);
    drive.setFlavor (aur::ADAASaturator::Flavor::Tube);
    volGain.reset (sr, 0.02);
}

bool PulseProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& out = layouts.getMainOutputChannelSet();
    return out == juce::AudioChannelSet::mono() || out == juce::AudioChannelSet::stereo();
}

void PulseProcessor::triggerVoice (int voice, float velocity)
{
    switch (voice)
    {
        case Drum::Kick:      kick.trigger (velocity); break;
        case Drum::Snare:     snare.trigger (velocity); break;
        case Drum::Clap:      clap.trigger (velocity); break;
        case Drum::ClosedHat: ohat.choke(); chat.trigger (velocity); break;   // closed chokes open
        case Drum::OpenHat:   ohat.trigger (velocity); break;
        case Drum::Tom:       tom.trigger (velocity); break;
        case Drum::Rim:       rim.trigger (velocity); break;
        default: break;
    }
}

float PulseProcessor::renderVoiceSample (int voice)
{
    switch (voice)
    {
        case Drum::Kick:      return kick.next();
        case Drum::Snare:     return snare.next();
        case Drum::Clap:      return clap.next();
        case Drum::ClosedHat: return chat.next();
        case Drum::OpenHat:   return ohat.next();
        case Drum::Tom:       return tom.next();
        case Drum::Rim:       return rim.next();
        default: return 0.0f;
    }
}

void PulseProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    const int totalOut = getTotalNumOutputChannels();
    const int n = buffer.getNumSamples();
    buffer.clear();

    // Update per-voice params once per block.
    kick.setParams  (*pTune[Drum::Kick],  *pDecay[Drum::Kick]);
    snare.setParams (*pTune[Drum::Snare], *pDecay[Drum::Snare]);
    clap.setParams  (*pTune[Drum::Clap],  *pDecay[Drum::Clap]);
    chat.setParams  (*pTune[Drum::ClosedHat], *pDecay[Drum::ClosedHat], false);
    ohat.setParams  (*pTune[Drum::OpenHat],   *pDecay[Drum::OpenHat],   true);
    tom.setParams   (*pTune[Drum::Tom],   *pDecay[Drum::Tom]);
    rim.setParams   (*pTune[Drum::Rim],   *pDecay[Drum::Rim]);

    // UI pad triggers (lock-free).
    for (int v = 0; v < Drum::Count; ++v)
        if (const int vel = uiTrigger[(size_t) v].exchange (0); vel > 0)
            triggerVoice (v, vel / 127.0f);

    // Pan: hats slightly right, rim slightly left, rest centred.
    auto panOf = [] (int v) { return v == Drum::ClosedHat || v == Drum::OpenHat ? 0.25f : (v == Drum::Rim ? -0.2f : 0.0f); };

    auto* L = buffer.getWritePointer (0);
    auto* R = totalOut > 1 ? buffer.getWritePointer (1) : nullptr;

    auto midiIt = midi.cbegin();
    const auto midiEnd = midi.cend();

    for (int s = 0; s < n; ++s)
    {
        while (midiIt != midiEnd && (*midiIt).samplePosition <= s)
        {
            const auto m = (*midiIt).getMessage();
            if (m.isNoteOn())
                for (int v = 0; v < Drum::Count; ++v)
                    if (m.getNoteNumber() == Drum::midiNote[(size_t) v])
                        triggerVoice (v, m.getFloatVelocity());
            ++midiIt;
        }

        float l = 0.0f, r = 0.0f;
        for (int v = 0; v < Drum::Count; ++v)
        {
            const float smp = renderVoiceSample (v) * (*pLevel[v]);
            const float pan = panOf (v);
            l += smp * (1.0f - juce::jmax (0.0f, pan));
            r += smp * (1.0f + juce::jmin (0.0f, pan));
        }
        L[s] = l;
        if (R) R[s] = r;
    }

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

juce::AudioProcessorEditor* PulseProcessor::createEditor() { return new PulseEditor (*this); }

int PulseProcessor::getNumPrograms() { return (int) Presets::getFactoryPresets().size(); }

void PulseProcessor::setCurrentProgram (int index)
{
    if (index < 0 || index >= getNumPrograms()) return;
    currentProgram = index;
    Presets::apply (index, apvts);
}

const juce::String PulseProcessor::getProgramName (int index)
{
    const auto& p = Presets::getFactoryPresets();
    return (index >= 0 && index < (int) p.size()) ? p[(size_t) index].name : juce::String{};
}

void PulseProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    if (auto state = apvts.copyState(); state.isValid())
    {
        juce::MemoryOutputStream mos (dest, false);
        state.writeToStream (mos);
    }
}

void PulseProcessor::setStateInformation (const void* data, int size)
{
    auto tree = juce::ValueTree::readFromData (data, (size_t) size);
    if (tree.isValid()) apvts.replaceState (tree);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new PulseProcessor(); }
