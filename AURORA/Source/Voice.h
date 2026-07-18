#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <atomic>
#include <vector>
#include "PolyBlepOsc.h"
#include "Parameters.h"

/** Raw-pointer snapshot of all synth parameters, read by every voice. */
struct SynthParams
{
    void bind (juce::AudioProcessorValueTreeState& s)
    {
        auto g = [&s] (const char* id) { return s.getRawParameterValue (id); };
        osc1shape = g(ParamID::osc1shape); osc1pw = g(ParamID::osc1pw); osc1level = g(ParamID::osc1level);
        osc2shape = g(ParamID::osc2shape); osc2pw = g(ParamID::osc2pw); osc2level = g(ParamID::osc2level);
        osc2coarse = g(ParamID::osc2coarse); osc2fine = g(ParamID::osc2fine);
        sublevel = g(ParamID::sublevel); noiselevel = g(ParamID::noiselevel);
        cutoff = g(ParamID::cutoff); resonance = g(ParamID::resonance); fdrive = g(ParamID::fdrive);
        envamt = g(ParamID::envamt); keytrack = g(ParamID::keytrack);
        fatk = g(ParamID::fatk); fdec = g(ParamID::fdec); fsus = g(ParamID::fsus); frel = g(ParamID::frel);
        aatk = g(ParamID::aatk); adec = g(ParamID::adec); asus = g(ParamID::asus); arel = g(ParamID::arel);
        unison = g(ParamID::unison); detune = g(ParamID::detune); spread = g(ParamID::spread);
        lforate = g(ParamID::lforate); lfoshape = g(ParamID::lfoshape);
        lfo2cut = g(ParamID::lfo2cut); lfo2pitch = g(ParamID::lfo2pitch);
        glide = g(ParamID::glide);
    }

    std::atomic<float> *osc1shape{}, *osc1pw{}, *osc1level{}, *osc2shape{}, *osc2pw{}, *osc2level{},
        *osc2coarse{}, *osc2fine{}, *sublevel{}, *noiselevel{}, *cutoff{}, *resonance{}, *fdrive{},
        *envamt{}, *keytrack{}, *fatk{}, *fdec{}, *fsus{}, *frel{}, *aatk{}, *adec{}, *asus{}, *arel{},
        *unison{}, *detune{}, *spread{}, *lforate{}, *lfoshape{}, *lfo2cut{}, *lfo2pitch{}, *glide{};
};

struct AuroraSound : juce::SynthesiserSound
{
    bool appliesToNote (int) override    { return true; }
    bool appliesToChannel (int) override { return true; }
};

/** One polyphonic voice: 2 unison-capable PolyBLEP oscillators + sub + noise,
    two ADSRs (amp & filter), and a ZDF ladder filter (JUCE), filtered in
    control-rate sub-blocks so the cutoff tracks the envelope + LFO. */
class AuroraVoice : public juce::SynthesiserVoice
{
public:
    explicit AuroraVoice (const SynthParams& p) : sp (p) {}

    void prepareVoice (double sr, int blockSize)
    {
        setCurrentPlaybackSampleRate (sr);
        blockCap = juce::jmax (32, blockSize);
        juce::dsp::ProcessSpec spec { sr, (juce::uint32) blockCap, 2 };
        ladder.prepare (spec);
        ladder.setEnabled (true);
        ladder.setMode (juce::dsp::LadderFilterMode::LPF24);
        for (auto& o : osc1) o.prepare (sr);
        for (auto& o : osc2) o.prepare (sr);
        subOsc.prepare (sr); subOsc.setShape (3);
        ampEnv.setSampleRate (sr);
        filtEnv.setSampleRate (sr);
        vbuf.setSize (2, blockCap);
        ampArr.assign ((size_t) blockCap, 0.0f);
        fenvArr.assign ((size_t) blockCap, 0.0f);
    }

    bool canPlaySound (juce::SynthesiserSound* s) override
    {
        return dynamic_cast<AuroraSound*> (s) != nullptr;
    }

    void startNote (int midiNote, float velocity, juce::SynthesiserSound*, int currentPitchWheel) override
    {
        note = midiNote;
        level = 0.2f + 0.8f * velocity;
        pitchBend = pitchWheelToSemitones (currentPitchWheel);

        const double target = noteHz (note);
        if (currentFreq <= 0.0 || *sp.glide < 1.0e-4f) currentFreq = target;
        targetFreq = target;

        const int U = numUnison();
        juce::Random r ((juce::int64) (midiNote * 2654435761u));
        for (int u = 0; u < U; ++u) { osc1[u].reset (r.nextDouble()); osc2[u].reset (r.nextDouble()); }
        subOsc.reset (0.0);
        lfoPhase = 0.0; sh = 0.0f;

        updateEnvParams();
        ampEnv.noteOn();
        filtEnv.noteOn();
    }

    void stopNote (float, bool allowTailOff) override
    {
        if (allowTailOff) { ampEnv.noteOff(); filtEnv.noteOff(); }
        else              { ampEnv.reset(); filtEnv.reset(); clearCurrentNote(); }
    }

    void pitchWheelMoved (int v) override { pitchBend = pitchWheelToSemitones (v); }
    void controllerMoved (int, int) override {}

    void renderNextBlock (juce::AudioBuffer<float>& out, int startSample, int numSamples) override
    {
        if (! ampEnv.isActive()) return;
        if (numSamples > blockCap) return;   // safety (host exceeded prepared size)

        updateEnvParams();
        const int   U        = numUnison();
        const int   sh1      = (int) *sp.osc1shape, sh2 = (int) *sp.osc2shape;
        const float pw1      = *sp.osc1pw,  pw2 = *sp.osc2pw;
        const float lvl1     = *sp.osc1level, lvl2 = *sp.osc2level;
        const float subLvl   = *sp.sublevel, noiseLvl = *sp.noiselevel;
        const float detuneCt = *sp.detune * 35.0f;
        const float spread   = *sp.spread;
        const float baseCut  = *sp.cutoff;
        const float res      = juce::jlimit (0.0f, 1.0f, *sp.resonance * 0.99f);
        const float fdrive   = *sp.fdrive;
        const float envAmt   = *sp.envamt;
        const float keytrk   = *sp.keytrack;
        const float lfo2cut  = *sp.lfo2cut, lfo2pit = *sp.lfo2pitch;
        const int   lfoSh    = (int) *sp.lfoshape;
        const double sr      = getSampleRate();

        lfoInc = (double) *sp.lforate / sr;
        const float glideMs   = *sp.glide * *sp.glide * 400.0f;
        const double glideCoef = glideMs < 0.1 ? 1.0 : 1.0 - std::exp (-1.0 / (0.001 * glideMs * sr));
        const float uNorm = 1.0f / std::sqrt ((float) U);

        auto* vL = vbuf.getWritePointer (0);
        auto* vR = vbuf.getWritePointer (1);
        float lfoAtBlockStart = 0.0f;

        // --- Pass 1: oscillators → vbuf, capture amp/filter envelopes per sample.
        for (int s = 0; s < numSamples; ++s)
        {
            currentFreq += (targetFreq - currentFreq) * glideCoef;
            const double bendMul = std::pow (2.0, pitchBend / 12.0);

            const float lfo = lfoValue (lfoSh);
            if (s == 0) lfoAtBlockStart = lfo;
            lfoPhase += lfoInc; if (lfoPhase >= 1.0) lfoPhase -= 1.0;

            const double pitchMod = std::pow (2.0, (double) (lfo2pit * lfo) * (2.0 / 12.0));
            const double f1base = currentFreq * bendMul * pitchMod;
            const double f2base = f1base * std::pow (2.0, ((double) (int) *sp.osc2coarse + (double) *sp.osc2fine * 0.01) / 12.0);

            float l = 0.0f, rr = 0.0f;
            for (int u = 0; u < U; ++u)
            {
                const float du = U > 1 ? ((float) u / (float) (U - 1) * 2.0f - 1.0f) : 0.0f;
                const double det = std::pow (2.0, (du * detuneCt) / 1200.0);
                const float pan = du * spread;
                const float gL = std::cos ((pan * 0.5f + 0.5f) * juce::MathConstants<float>::halfPi);
                const float gR = std::sin ((pan * 0.5f + 0.5f) * juce::MathConstants<float>::halfPi);

                osc1[u].setShape (sh1); osc1[u].setPulseWidth (pw1); osc1[u].setFrequency (f1base * det);
                osc2[u].setShape (sh2); osc2[u].setPulseWidth (pw2); osc2[u].setFrequency (f2base * det);
                const float v = (osc1[u].next() * lvl1 + osc2[u].next() * lvl2) * uNorm;
                l += v * gL; rr += v * gR;
            }
            if (subLvl > 0.0f)   { subOsc.setFrequency (f1base * 0.5); const float sub = subOsc.next() * subLvl; l += sub; rr += sub; }
            if (noiseLvl > 0.0f) { const float nz = (rng.nextFloat() * 2.0f - 1.0f) * noiseLvl; l += nz; rr += nz; }

            vL[s] = l; vR[s] = rr;
            ampArr[(size_t) s]  = ampEnv.getNextSample();
            fenvArr[(size_t) s] = filtEnv.getNextSample();
        }

        // --- Pass 2: ladder filter in sub-blocks, cutoff from the envelope + LFO.
        ladder.setResonance (res);
        ladder.setDrive (fdrive);
        const float ktOct = keytrk * (float) (note - 60) / 12.0f;
        juce::dsp::AudioBlock<float> blk (vbuf);
        constexpr int SB = 16;
        for (int off = 0; off < numSamples; off += SB)
        {
            const int len = juce::jmin (SB, numSamples - off);
            const float fEnv = fenvArr[(size_t) off];
            const float modOct = envAmt * fEnv * 5.0f + lfo2cut * lfoAtBlockStart * 4.0f + ktOct;
            const float cut = juce::jlimit (20.0f, 20000.0f, baseCut * std::pow (2.0f, modOct));
            ladder.setCutoffFrequencyHz (cut);
            auto sub = blk.getSubBlock ((size_t) off, (size_t) len);
            juce::dsp::ProcessContextReplacing<float> ctx (sub);
            ladder.process (ctx);
        }

        // --- Pass 3: amp envelope → output.
        for (int s = 0; s < numSamples; ++s)
        {
            const float a = ampArr[(size_t) s] * level;
            out.addSample (0, startSample + s, vL[s] * a);
            if (out.getNumChannels() > 1) out.addSample (1, startSample + s, vR[s] * a);
        }

        if (! ampEnv.isActive())
        {
            clearCurrentNote();
            ladder.reset();
        }
    }

private:
    int  numUnison() const { return juce::jlimit (1, 7, (int) *sp.unison); }
    static double noteHz (int n) { return 440.0 * std::pow (2.0, (n - 69) / 12.0); }
    static double pitchWheelToSemitones (int w) { return (w - 8192) / 8192.0 * 2.0; }

    void updateEnvParams()
    {
        ampEnv.setParameters ({ *sp.aatk, *sp.adec, *sp.asus, *sp.arel });
        filtEnv.setParameters ({ *sp.fatk, *sp.fdec, *sp.fsus, *sp.frel });
    }

    float lfoValue (int shape)
    {
        const double t = lfoPhase;
        switch (shape)
        {
            case 1: return (float) (1.0 - 4.0 * std::abs (t - 0.5));
            case 2: return (float) (2.0 * t - 1.0);
            case 3: return t < 0.5 ? 1.0f : -1.0f;
            case 4: if (t < lfoInc) sh = rng.nextFloat() * 2.0f - 1.0f; return sh;
            default: return (float) std::sin (6.283185307179586 * t);
        }
    }

    const SynthParams& sp;
    aur::syn::PolyBlepOsc osc1[7], osc2[7], subOsc;
    juce::dsp::LadderFilter<float> ladder;
    juce::ADSR ampEnv, filtEnv;
    juce::Random rng;

    juce::AudioBuffer<float> vbuf;
    std::vector<float> ampArr, fenvArr;
    int blockCap = 512;

    int    note = 60;
    float  level = 1.0f;
    double pitchBend = 0.0;
    double currentFreq = 0.0, targetFreq = 440.0;
    double lfoPhase = 0.0, lfoInc = 0.0;
    float  sh = 0.0f;
};
