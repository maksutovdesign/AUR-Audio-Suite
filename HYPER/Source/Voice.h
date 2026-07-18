#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <atomic>
#include "WavetableOsc.h"
#include "SvfZDF.h"
#include "Parameters.h"

struct SynthParams
{
    void bind (juce::AudioProcessorValueTreeState& s)
    {
        auto g = [&s](const char* id){ return s.getRawParameterValue (id); };
        morph1=g(ParamID::morph1); morph2=g(ParamID::morph2); o2coarse=g(ParamID::o2coarse); o2level=g(ParamID::o2level);
        fm=g(ParamID::fm); unison=g(ParamID::unison); detune=g(ParamID::detune); spread=g(ParamID::spread);
        cutoff=g(ParamID::cutoff); reso=g(ParamID::reso); fmode=g(ParamID::fmode); envamt=g(ParamID::envamt);
        fatk=g(ParamID::fatk); fdec=g(ParamID::fdec); fsus=g(ParamID::fsus); frel=g(ParamID::frel);
        aatk=g(ParamID::aatk); adec=g(ParamID::adec); asus=g(ParamID::asus); arel=g(ParamID::arel);
        lforate=g(ParamID::lforate); lfo2cut=g(ParamID::lfo2cut); lfo2morph=g(ParamID::lfo2morph);
        glide=g(ParamID::glide);
    }
    std::atomic<float> *morph1{},*morph2{},*o2coarse{},*o2level{},*fm{},*unison{},*detune{},*spread{},
        *cutoff{},*reso{},*fmode{},*envamt{},*fatk{},*fdec{},*fsus{},*frel{},*aatk{},*adec{},*asus{},*arel{},
        *lforate{},*lfo2cut{},*lfo2morph{},*glide{};
};

struct HyperSound : juce::SynthesiserSound
{
    bool appliesToNote (int) override { return true; }
    bool appliesToChannel (int) override { return true; }
};

/** Flagship hybrid voice: unison wavetable osc1 (≤5, detune+spread) + wavetable
    osc2 (coarse) with FM 2→1, morphing tables, multimode SVF with its own env,
    LFO → cutoff & morph, portamento. */
class HyperVoice : public juce::SynthesiserVoice
{
public:
    explicit HyperVoice (const SynthParams& p) : sp (p) {}
    void prepareVoice (double sr)
    {
        setCurrentPlaybackSampleRate (sr);
        for (auto& o : o1) o.prepare (sr);
        o2.prepare (sr);
        fL.prepare (sr); fR.prepare (sr);
        aEnv.setSampleRate (sr); fEnv.setSampleRate (sr);
    }
    bool canPlaySound (juce::SynthesiserSound* s) override { return dynamic_cast<HyperSound*> (s) != nullptr; }

    void startNote (int note, float vel, juce::SynthesiserSound*, int wheel) override
    {
        level = 0.22f + 0.78f * vel;
        bend = (wheel - 8192) / 8192.0 * 2.0;
        const double t = 440.0 * std::pow (2.0, (note - 69) / 12.0);
        if (freq <= 0.0 || *sp.glide < 1.0e-4f) freq = t;
        target = t;
        juce::Random r ((juce::int64) note * 9973);
        for (auto& o : o1) o.reset (r.nextDouble());
        o2.reset (r.nextDouble());
        lfoPhase = 0.0;
        updateEnvs();
        aEnv.noteOn(); fEnv.noteOn();
    }
    void stopNote (float, bool tail) override
    {
        if (tail) { aEnv.noteOff(); fEnv.noteOff(); }
        else { aEnv.reset(); fEnv.reset(); clearCurrentNote(); }
    }
    void pitchWheelMoved (int v) override { bend = (v - 8192) / 8192.0 * 2.0; }
    void controllerMoved (int, int) override {}

    void renderNextBlock (juce::AudioBuffer<float>& out, int start, int n) override
    {
        if (! aEnv.isActive()) return;
        updateEnvs();
        const int U = juce::jlimit (1, 5, (int) *sp.unison);
        const float m1 = *sp.morph1, m2 = *sp.morph2;
        const float o2lvl = *sp.o2level, fmAmt = *sp.fm;
        const float det = *sp.detune * 40.0f, spr = *sp.spread;
        const float base = *sp.cutoff, res = *sp.reso, envAmt = *sp.envamt;
        const int   fmode = (int) *sp.fmode;
        const float l2c = *sp.lfo2cut, l2m = *sp.lfo2morph;
        const double sr = getSampleRate();
        const double lfoInc = (double) *sp.lforate / sr;
        const float glideMs = *sp.glide * *sp.glide * 400.0f;
        const double gc = glideMs < 0.1 ? 1.0 : 1.0 - std::exp (-1.0 / (0.001 * glideMs * sr));
        const float uNorm = 1.0f / std::sqrt ((float) U);

        for (int s = 0; s < n; ++s)
        {
            freq += (target - freq) * gc;
            const double f0 = freq * std::pow (2.0, bend / 12.0);

            lfoPhase += lfoInc; if (lfoPhase >= 1.0) lfoPhase -= 1.0;
            const float lfo = (float) std::sin (6.28318530718 * lfoPhase);

            const float mo = juce::jlimit (0.0f, 1.0f, m1 + l2m * 0.5f * lfo);
            const float fe = fEnv.getNextSample();

            // Osc2 (also FM source).
            o2.setMorph (m2);
            o2.setFrequency (f0 * std::pow (2.0, (double) (int) *sp.o2coarse / 12.0));
            const float v2 = o2.next();

            // Osc1 unison, frequency-modulated by osc2.
            const double fmMul = std::pow (2.0, (double) (fmAmt * v2) * 0.8);
            float l = 0.f, r = 0.f;
            for (int u = 0; u < U; ++u)
            {
                const float du = U > 1 ? ((float) u / (float) (U - 1) * 2.0f - 1.0f) : 0.0f;
                o1[u].setMorph (mo);
                o1[u].setFrequency (f0 * fmMul * std::pow (2.0, (double) (du * det) / 1200.0));
                const float v = o1[u].next() * uNorm;
                const float pan = du * spr;
                l += v * (0.5f - 0.5f * pan);
                r += v * (0.5f + 0.5f * pan);
            }
            l += v2 * o2lvl * 0.5f; r += v2 * o2lvl * 0.5f;

            if ((ctrl++ & 15) == 0)
            {
                const float cut = juce::jlimit (20.0f, 20000.0f,
                    base * std::pow (2.0f, envAmt * fe * 5.0f + l2c * lfo * 4.0f));
                fL.set (cut, res); fR.set (cut, res);
            }
            const float a = aEnv.getNextSample() * level;
            out.addSample (0, start + s, fL.process (l, fmode) * a);
            if (out.getNumChannels() > 1) out.addSample (1, start + s, fR.process (r, fmode) * a);
        }
        if (! aEnv.isActive()) { clearCurrentNote(); fL.reset(); fR.reset(); }
    }

private:
    void updateEnvs()
    {
        aEnv.setParameters ({ *sp.aatk, *sp.adec, *sp.asus, *sp.arel });
        fEnv.setParameters ({ *sp.fatk, *sp.fdec, *sp.fsus, *sp.frel });
    }
    const SynthParams& sp;
    aur::syn::WavetableOsc o1[5], o2;
    aur::syn::SvfZDF fL, fR;
    juce::ADSR aEnv, fEnv;
    float level = 1.0f;
    double bend = 0.0, freq = 0.0, target = 440.0, lfoPhase = 0.0;
    unsigned ctrl = 0;
};
