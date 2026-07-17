#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Presets.h"
#include <cmath>

ConvoProcessor::ConvoProcessor()
    : AudioProcessor (BusesProperties()
        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMETERS", Params::createLayout())
{
    pDecay    = apvts.getRawParameterValue (ParamID::decay);
    pTone     = apvts.getRawParameterValue (ParamID::tone);
    pPredelay = apvts.getRawParameterValue (ParamID::predelay);
    pWidth    = apvts.getRawParameterValue (ParamID::width);
    pMix      = apvts.getRawParameterValue (ParamID::mix);
    pBypass   = apvts.getRawParameterValue (ParamID::bypass);
}

void ConvoProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    numChannels       = juce::jmax (1, getTotalNumOutputChannels());

    juce::dsp::ProcessSpec spec { sampleRate,
                                  (juce::uint32) juce::jmax (1, samplesPerBlock),
                                  (juce::uint32) numChannels };
    conv.prepare (spec);

    wetBuffer.setSize (numChannels, samplesPerBlock);
    dryBuffer.setSize (numChannels, samplesPerBlock);
    mixSmooth.reset (sampleRate, 0.02);
    setLatencySamples (0);

    // Force a rebuild for the current parameter values.
    lastDecay = lastTone = lastPredelay = lastWidth = -1.f;
    rebuildImpulseResponse();
}

bool ConvoProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& out = layouts.getMainOutputChannelSet();
    if (out != juce::AudioChannelSet::mono() && out != juce::AudioChannelSet::stereo())
        return false;
    return layouts.getMainInputChannelSet() == out;
}

void ConvoProcessor::rebuildImpulseResponse()
{
    const double sr       = currentSampleRate;
    const float  decaySec = pDecay->load();
    const float  tone     = pTone->load()  * 0.01f;   // 0..1
    const float  preMs    = pPredelay->load();
    const float  width    = pWidth->load() * 0.01f;   // 0..1

    lastDecay = decaySec; lastTone = tone; lastPredelay = preMs; lastWidth = width;

    const int preSamples  = juce::jmax (0, (int) std::round (preMs * 0.001 * sr));
    const int tailSamples = juce::jmax (16, (int) std::round (decaySec * sr));
    const int total       = preSamples + tailSamples;

    juce::AudioBuffer<float> ir (2, total);
    ir.clear();

    // Tone → one-pole lowpass cutoff. Darker (low tone) damps highs harder.
    const float cutoffHz = juce::jmap (tone, 0.f, 1.f, 900.f, 16000.f);
    const float lpCoef   = 1.0f - std::exp (-2.0f * juce::MathConstants<float>::pi
                                            * cutoffHz / (float) sr);

    // Two independent noise sources; R is blended toward L by (1-width).
    juce::Random rngL (0x51ed270b), rngR (0x2a13f9c7);
    const float decayK = 6.9077553f / (float) tailSamples; // ln(1000) → -60 dB at the tail end

    float lpL = 0.f, lpR = 0.f;
    auto* wL = ir.getWritePointer (0);
    auto* wR = ir.getWritePointer (1);

    for (int i = 0; i < tailSamples; ++i)
    {
        const float env = std::exp (-decayK * (float) i);
        const float nL = rngL.nextFloat() * 2.0f - 1.0f;
        const float nR = rngR.nextFloat() * 2.0f - 1.0f;
        lpL += lpCoef * (nL - lpL);
        lpR += lpCoef * (nR - lpR);
        const int idx = preSamples + i;
        wL[idx] = lpL * env;
        wR[idx] = (lpR * width + lpL * (1.0f - width)) * env;
    }

    // Normalise to unity energy so loudness stays consistent as Decay/Tone move;
    // the Mix knob then sets the audible amount.
    double e = 0.0;
    for (int c = 0; c < 2; ++c)
    {
        const auto* r = ir.getReadPointer (c);
        for (int i = 0; i < total; ++i) e += (double) r[i] * r[i];
    }
    const float norm = e > 1.0e-9 ? (float) (1.0 / std::sqrt (e)) : 1.0f;
    ir.applyGain (norm);

    conv.loadImpulseResponse (std::move (ir), sr,
                              juce::dsp::Convolution::Stereo::yes,
                              juce::dsp::Convolution::Trim::no,
                              juce::dsp::Convolution::Normalise::no);
}

void ConvoProcessor::handleAsyncUpdate() { rebuildImpulseResponse(); }

void ConvoProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const int totalIn  = getTotalNumInputChannels();
    const int totalOut = getTotalNumOutputChannels();
    const int n        = buffer.getNumSamples();
    for (int ch = totalIn; ch < totalOut; ++ch)
        buffer.clear (ch, 0, n);

    meters.pushInputPeak (buffer.getMagnitude (0, n));

    // Ask for an IR rebuild (message thread) if any shaping value moved.
    const float eps = 1.0e-4f;
    if (std::abs (pDecay->load()          - lastDecay)    > eps
        || std::abs (pTone->load() * 0.01f - lastTone)    > eps
        || std::abs (pPredelay->load()     - lastPredelay) > eps
        || std::abs (pWidth->load() * 0.01f - lastWidth)  > eps)
    {
        // Latch immediately so we don't re-trigger every block while pending.
        lastDecay = pDecay->load(); lastTone = pTone->load() * 0.01f;
        lastPredelay = pPredelay->load(); lastWidth = pWidth->load() * 0.01f;
        triggerAsyncUpdate();
    }

    const bool bypassed = *pBypass > 0.5f;
    mixSmooth.setTargetValue (bypassed ? 0.0f : pMix->load() * 0.01f);

    dryBuffer.makeCopyOf (buffer, true);

    // Wet path: run a copy through the convolver.
    wetBuffer.makeCopyOf (buffer, true);
    juce::dsp::AudioBlock<float> block (wetBuffer);
    juce::dsp::ProcessContextReplacing<float> ctx (block);
    conv.process (ctx);

    for (int s = 0; s < n; ++s)
    {
        const float m   = mixSmooth.getNextValue();
        const float dry = 1.0f - m;
        for (int ch = 0; ch < totalOut; ++ch)
        {
            auto* out       = buffer.getWritePointer (ch);
            const auto* d   = dryBuffer.getReadPointer (ch);
            const auto* wet = wetBuffer.getReadPointer (juce::jmin (ch, wetBuffer.getNumChannels() - 1));
            out[s] = d[s] * dry + wet[s] * m;
        }
    }

    meters.pushOutputPeak (buffer.getMagnitude (0, n));
}

juce::AudioProcessorEditor* ConvoProcessor::createEditor() { return new ConvoEditor (*this); }

int ConvoProcessor::getNumPrograms() { return (int) Presets::getFactoryPresets().size(); }

void ConvoProcessor::setCurrentProgram (int index)
{
    if (index < 0 || index >= getNumPrograms()) return;
    currentProgram = index;
    Presets::apply (index, apvts);
}

const juce::String ConvoProcessor::getProgramName (int index)
{
    const auto& p = Presets::getFactoryPresets();
    return (index >= 0 && index < (int) p.size()) ? p[(size_t) index].name : juce::String{};
}

void ConvoProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    if (auto state = apvts.copyState(); state.isValid())
    {
        juce::MemoryOutputStream mos (dest, false);
        state.writeToStream (mos);
    }
}

void ConvoProcessor::setStateInformation (const void* data, int size)
{
    auto tree = juce::ValueTree::readFromData (data, (size_t) size);
    if (tree.isValid()) apvts.replaceState (tree);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new ConvoProcessor(); }
