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
        morph=g(ParamID::morph); lfoamt=g(ParamID::lfoamt); lforate=g(ParamID::lforate); detune=g(ParamID::detune);
        cutoff=g(ParamID::cutoff); reso=g(ParamID::reso); envamt=g(ParamID::envamt);
        attack=g(ParamID::attack); decay=g(ParamID::decay); sustain=g(ParamID::sustain); release=g(ParamID::release);
    }
    std::atomic<float> *morph{},*lfoamt{},*lforate{},*detune{},*cutoff{},*reso{},*envamt{},
        *attack{},*decay{},*sustain{},*release{};
};

struct WaveSound : juce::SynthesiserSound
{
    bool appliesToNote (int) override { return true; }
    bool appliesToChannel (int) override { return true; }
};

/** Morphing wavetable voice: two detuned band-limited wavetable oscillators
    (sine→tri→saw→square continuum) with LFO morph motion → SVF. */
class WaveVoice : public juce::SynthesiserVoice
{
public:
    explicit WaveVoice (const SynthParams& p) : sp (p) {}
    void prepareVoice (double sr)
    {
        setCurrentPlaybackSampleRate (sr);
        o1.prepare (sr); o2.prepare (sr);
        filt.prepare (sr);
        env.setSampleRate (sr);
    }
    bool canPlaySound (juce::SynthesiserSound* s) override { return dynamic_cast<WaveSound*> (s) != nullptr; }

    void startNote (int note, float vel, juce::SynthesiserSound*, int wheel) override
    {
        level = 0.25f + 0.75f * vel;
        bend = (wheel - 8192) / 8192.0 * 2.0;
        freq = 440.0 * std::pow (2.0, (note - 69) / 12.0);
        juce::Random r ((juce::int64) note * 6151);
        o1.reset (r.nextDouble()); o2.reset (r.nextDouble());
        lfoPhase = 0.0;
        env.setParameters ({ *sp.attack, *sp.decay, *sp.sustain, *sp.release });
        env.noteOn();
    }
    void stopNote (float, bool tail) override { if (tail) env.noteOff(); else { env.reset(); clearCurrentNote(); } }
    void pitchWheelMoved (int v) override { bend = (v - 8192) / 8192.0 * 2.0; }
    void controllerMoved (int, int) override {}

    void renderNextBlock (juce::AudioBuffer<float>& out, int start, int n) override
    {
        if (! env.isActive()) return;
        env.setParameters ({ *sp.attack, *sp.decay, *sp.sustain, *sp.release });
        const float morph = *sp.morph, lfoA = *sp.lfoamt, det = *sp.detune * 12.0f;
        const float base = *sp.cutoff, res = *sp.reso, envAmt = *sp.envamt;
        const double sr = getSampleRate();
        const double lfoInc = (double) *sp.lforate / sr;
        const double f = freq * std::pow (2.0, bend / 12.0);
        o1.setFrequency (f * std::pow (2.0, -det / 2400.0));
        o2.setFrequency (f * std::pow (2.0,  det / 2400.0));

        for (int s = 0; s < n; ++s)
        {
            lfoPhase += lfoInc; if (lfoPhase >= 1.0) lfoPhase -= 1.0;
            const float lfo = (float) std::sin (6.28318530718 * lfoPhase);
            const float m = juce::jlimit (0.0f, 1.0f, morph + lfoA * 0.5f * lfo);
            o1.setMorph (m); o2.setMorph (m);

            const float e = env.getNextSample();
            float v = (o1.next() + o2.next()) * 0.5f;
            if ((ctrl++ & 15) == 0)
                filt.set (juce::jlimit (20.0f, 20000.0f, base * std::pow (2.0f, envAmt * e * 4.0f)), res);
            v = filt.process (v, 0) * e * level;
            out.addSample (0, start + s, v);
            if (out.getNumChannels() > 1) out.addSample (1, start + s, v);
        }
        if (! env.isActive()) { clearCurrentNote(); filt.reset(); }
    }

private:
    const SynthParams& sp;
    aur::syn::WavetableOsc o1, o2;
    aur::syn::SvfZDF filt;
    juce::ADSR env;
    float level = 1.0f;
    double bend = 0.0, freq = 440.0, lfoPhase = 0.0;
    unsigned ctrl = 0;
};
