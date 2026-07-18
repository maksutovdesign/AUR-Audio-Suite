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
    pChar     = apvts.getRawParameterValue (ParamID::character);
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

    predelay.setMaximumDelayInSamples ((int) std::ceil (0.16 * sampleRate) + 4);
    predelay.prepare (spec);
    predelay.reset();

    wetBuffer.setSize (numChannels, samplesPerBlock);
    dryBuffer.setSize (numChannels, samplesPerBlock);
    mixSmooth.reset (sampleRate, 0.02);
    setLatencySamples (0);

    // Restore whatever IR source the saved state asked for.
    const auto savedPath = apvts.state.getProperty (kIRPathProp).toString();
    if (savedPath.isNotEmpty() && juce::File (savedPath).existsAsFile())
        loadImpulseFile (juce::File (savedPath));
    else
    {
        lastDecay = lastTone = lastWidth = lastChar = -1.f;   // force a rebuild
        rebuildSyntheticIR();
    }
}

bool ConvoProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& out = layouts.getMainOutputChannelSet();
    if (out != juce::AudioChannelSet::mono() && out != juce::AudioChannelSet::stereo())
        return false;
    return layouts.getMainInputChannelSet() == out;
}

namespace
{
    // Schroeder all-pass — used to diffuse (smear) the noise into a denser,
    // more natural tail. Delay in samples, feedback g.
    struct AllPass
    {
        std::vector<float> buf;
        int idx = 0; float g = 0.5f;
        void init (int delaySamples, float feedback)
        {
            buf.assign ((size_t) juce::jmax (1, delaySamples), 0.0f);
            idx = 0; g = feedback;
        }
        inline float process (float x)
        {
            const float d = buf[(size_t) idx];
            const float y = -g * x + d;
            buf[(size_t) idx] = x + g * y;
            if (++idx >= (int) buf.size()) idx = 0;
            return y;
        }
    };
}

void ConvoProcessor::rebuildSyntheticIR()
{
    const double sr       = currentSampleRate;
    const int    character = (int) std::round (pChar->load());
    const float  tone     = pTone->load()  * 0.01f;   // 0..1
    const float  width    = pWidth->load() * 0.01f;   // 0..1
    float        decaySec = pDecay->load();

    lastDecay = pDecay->load(); lastTone = tone; lastWidth = width;
    lastChar  = (float) character;

    // Per-character shaping.
    //   lenScale   — stretches/compresses the tail length
    //   brightMul  — scales the starting HF cutoff
    //   dampEnd    — HF cutoff fraction reached at the tail end (frequency-
    //                dependent decay: highs die faster than lows)
    //   buildup    — fraction of the tail over which energy ramps up (halls)
    //   diffusion  — number of all-pass smearing passes
    //   erGain     — level of the discrete early reflections
    //   disperse   — extra dispersive all-pass (spring "boing")
    float lenScale = 1.0f, brightMul = 1.0f, dampEnd = 0.35f, buildup = 0.0f, erGain = 0.0f;
    int   diffusion = 2; bool disperse = false;
    switch (character)
    {
        case 1: lenScale = 0.7f; brightMul = 1.05f; dampEnd = 0.45f; erGain = 0.7f;  diffusion = 2; break; // Room
        case 2: lenScale = 1.0f; brightMul = 1.4f;  dampEnd = 0.7f;  erGain = 0.0f;  diffusion = 5; break; // Plate
        case 3: lenScale = 1.3f; brightMul = 0.95f; dampEnd = 0.25f; buildup = 0.18f; erGain = 0.35f; diffusion = 3; break; // Hall
        case 4: lenScale = 0.9f; brightMul = 1.1f;  dampEnd = 0.5f;  erGain = 0.2f;  diffusion = 1; disperse = true; break; // Spring
        default: break; // Smooth
    }

    decaySec = juce::jlimit (0.05f, 8.0f, decaySec * lenScale);
    const int tailSamples = juce::jmax (16, (int) std::round (decaySec * sr));

    juce::AudioBuffer<float> ir (2, tailSamples);
    ir.clear();

    const float cutoffStart = juce::jlimit (400.f, 18000.f,
                                            juce::jmap (tone, 0.f, 1.f, 900.f, 16000.f) * brightMul);
    const float cutoffEnd   = juce::jmax (300.f, cutoffStart * dampEnd);

    juce::Random rngL (0x51ed270b), rngR (0x2a13f9c7);
    const float decayK = 6.9077553f / (float) tailSamples; // ln(1000) → -60 dB at the tail end

    float lpL = 0.f, lpR = 0.f;
    auto* wL = ir.getWritePointer (0);
    auto* wR = ir.getWritePointer (1);

    for (int i = 0; i < tailSamples; ++i)
    {
        const float frac = (float) i / (float) tailSamples;
        // Frequency-dependent damping: cutoff glides from bright → dark.
        const float cutoff = cutoffStart + (cutoffEnd - cutoffStart) * frac;
        const float lpCoef = 1.0f - std::exp (-2.0f * juce::MathConstants<float>::pi
                                              * cutoff / (float) sr);
        float env = std::exp (-decayK * (float) i);
        if (buildup > 0.0f && frac < buildup)            // halls swell in
            env *= frac / buildup;

        const float nL = rngL.nextFloat() * 2.0f - 1.0f;
        const float nR = rngR.nextFloat() * 2.0f - 1.0f;
        lpL += lpCoef * (nL - lpL);
        lpR += lpCoef * (nR - lpR);
        wL[i] = lpL * env;
        wR[i] = (lpR * width + lpL * (1.0f - width)) * env;
    }

    // Diffusion: smear the noise through a short all-pass chain per channel.
    if (diffusion > 0)
    {
        const int primes[6] = { 142, 107, 379, 277, 449, 193 };
        for (int c = 0; c < 2; ++c)
        {
            auto* w = ir.getWritePointer (c);
            for (int d = 0; d < diffusion; ++d)
            {
                AllPass ap; ap.init (primes[d % 6] + c * 13, 0.6f);
                for (int i = 0; i < tailSamples; ++i) w[i] = ap.process (w[i]);
            }
            if (disperse)   // spring: long dispersive all-pass → "boing"
            {
                AllPass sp; sp.init ((int) (0.028 * sr) + c * 7, 0.72f);
                for (int i = 0; i < tailSamples; ++i) w[i] = sp.process (w[i]);
            }
        }
    }

    // Discrete early reflections at the head of the IR.
    if (erGain > 0.0f)
    {
        const float ms[6]   = { 7.f, 13.f, 19.f, 23.f, 29.f, 37.f };
        const float gain[6] = { 0.9f, -0.75f, 0.6f, -0.5f, 0.42f, -0.34f };
        for (int c = 0; c < 2; ++c)
        {
            auto* w = ir.getWritePointer (c);
            for (int t = 0; t < 6; ++t)
            {
                const int idx = (int) std::round (ms[t] * 0.001f * sr) + c * 3;
                if (idx < tailSamples) w[idx] += erGain * gain[t];
            }
        }
    }

    // Normalise to unity energy so loudness stays consistent; Mix sets amount.
    double e = 0.0;
    for (int c = 0; c < 2; ++c)
    {
        const auto* r = ir.getReadPointer (c);
        for (int i = 0; i < tailSamples; ++i) e += (double) r[i] * r[i];
    }
    const float norm = e > 1.0e-9 ? (float) (1.0 / std::sqrt (e)) : 1.0f;
    ir.applyGain (norm);

    conv.loadImpulseResponse (std::move (ir), sr,
                              juce::dsp::Convolution::Stereo::yes,
                              juce::dsp::Convolution::Trim::no,
                              juce::dsp::Convolution::Normalise::no);
    usingFile.store (false);
}

void ConvoProcessor::loadImpulseFile (const juce::File& file)
{
    if (! file.existsAsFile()) return;

    conv.loadImpulseResponse (file,
                              juce::dsp::Convolution::Stereo::yes,
                              juce::dsp::Convolution::Trim::yes,
                              0,                                     // load whole file
                              juce::dsp::Convolution::Normalise::yes);
    usingFile.store (true);
    apvts.state.setProperty (kIRPathProp, file.getFullPathName(), nullptr);
}

void ConvoProcessor::useSyntheticIR()
{
    apvts.state.setProperty (kIRPathProp, "", nullptr);
    lastDecay = lastTone = lastWidth = -1.f;   // force a rebuild
    rebuildSyntheticIR();
}

juce::String ConvoProcessor::getIRSourceName() const
{
    if (usingFile.load())
    {
        const auto p = apvts.state.getProperty (kIRPathProp).toString();
        return p.isNotEmpty() ? juce::File (p).getFileName() : juce::String ("File");
    }
    static const char* names[5] = { "Smooth", "Room", "Plate", "Hall", "Spring" };
    const int c = juce::jlimit (0, 4, (int) std::round (pChar->load()));
    return juce::String ("Synthetic · ") + names[c];
}

void ConvoProcessor::handleAsyncUpdate() { rebuildSyntheticIR(); }

void ConvoProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const int totalIn  = getTotalNumInputChannels();
    const int totalOut = getTotalNumOutputChannels();
    const int n        = buffer.getNumSamples();
    for (int ch = totalIn; ch < totalOut; ++ch)
        buffer.clear (ch, 0, n);

    meters.pushInputPeak (buffer.getMagnitude (0, n));

    // Ask for a synthetic-IR rebuild (message thread) if a shaping value moved —
    // but only while the synthetic source is active (never clobber a loaded file).
    if (! usingFile.load())
    {
        const float eps = 1.0e-4f;
        if (std::abs (pDecay->load()           - lastDecay) > eps
            || std::abs (pTone->load()  * 0.01f - lastTone) > eps
            || std::abs (pWidth->load() * 0.01f - lastWidth) > eps
            || std::abs (pChar->load()          - lastChar) > 0.5f)
        {
            lastDecay = pDecay->load();
            lastTone  = pTone->load()  * 0.01f;
            lastWidth = pWidth->load() * 0.01f;
            lastChar  = pChar->load();
            triggerAsyncUpdate();
        }
    }

    const bool bypassed = *pBypass > 0.5f;
    mixSmooth.setTargetValue (bypassed ? 0.0f : pMix->load() * 0.01f);

    dryBuffer.makeCopyOf (buffer, true);

    // Wet path: convolve a copy, then apply the pre-delay line.
    wetBuffer.makeCopyOf (buffer, true);
    juce::dsp::AudioBlock<float> block (wetBuffer);
    juce::dsp::ProcessContextReplacing<float> ctx (block);
    conv.process (ctx);

    const float preSamples = juce::jlimit (0.0f, (float) predelay.getMaximumDelayInSamples() - 1.0f,
                                           pPredelay->load() * 0.001f * (float) currentSampleRate);
    predelay.setDelay (preSamples);
    for (int ch = 0; ch < totalOut; ++ch)
    {
        auto* w = wetBuffer.getWritePointer (juce::jmin (ch, wetBuffer.getNumChannels() - 1));
        for (int s = 0; s < n; ++s)
        {
            predelay.pushSample (ch, w[s]);
            w[s] = predelay.popSample (ch);
        }
    }

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
    if (! tree.isValid()) return;
    apvts.replaceState (tree);

    const auto path = apvts.state.getProperty (kIRPathProp).toString();
    if (path.isNotEmpty() && juce::File (path).existsAsFile())
        loadImpulseFile (juce::File (path));
    else
        useSyntheticIR();
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new ConvoProcessor(); }
