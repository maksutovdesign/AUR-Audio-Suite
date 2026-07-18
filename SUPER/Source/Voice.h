#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <atomic>
#include "PolyBlepOsc.h"
#include "SvfZDF.h"
#include "Parameters.h"

struct SynthParams
{
    void bind (juce::AudioProcessorValueTreeState& s)
    {
        auto g = [&s](const char* id){ return s.getRawParameterValue (id); };
        detune=g(ParamID::detune); spread=g(ParamID::spread); mixOsc=g(ParamID::mixOsc);
        cutoff=g(ParamID::cutoff); reso=g(ParamID::reso);
        attack=g(ParamID::attack); decay=g(ParamID::decay); sustain=g(ParamID::sustain); release=g(ParamID::release);
        glide=g(ParamID::glide);
    }
    std::atomic<float> *detune{},*spread{},*mixOsc{},*cutoff{},*reso{},
        *attack{},*decay{},*sustain{},*release{},*glide{};
};

struct SuperSound : juce::SynthesiserSound
{
    bool appliesToNote (int) override { return true; }
    bool appliesToChannel (int) override { return true; }
};

/** Supersaw: 7 detuned PolyBLEP saws (1 centre + 6 sides), stereo spread,
    stereo SVF lowpass. The classic trance/hyper-pop stack. */
class SuperVoice : public juce::SynthesiserVoice
{
public:
    explicit SuperVoice (const SynthParams& p) : sp (p) {}
    void prepareVoice (double sr)
    {
        setCurrentPlaybackSampleRate (sr);
        for (auto& o : osc) o.prepare (sr);
        fL.prepare (sr); fR.prepare (sr);
        env.setSampleRate (sr);
    }
    bool canPlaySound (juce::SynthesiserSound* s) override { return dynamic_cast<SuperSound*> (s) != nullptr; }

    void startNote (int note, float vel, juce::SynthesiserSound*, int wheel) override
    {
        level = 0.2f + 0.8f * vel;
        bend = (wheel - 8192) / 8192.0 * 2.0;
        const double t = 440.0 * std::pow (2.0, (note - 69) / 12.0);
        if (freq <= 0.0 || *sp.glide < 1.0e-4f) freq = t;
        target = t;
        juce::Random r ((juce::int64) note * 7919);
        for (auto& o : osc) { o.setShape (0); o.reset (r.nextDouble()); }
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
        const float det = *sp.detune * 60.0f;   // cents span
        const float spr = *sp.spread, mix = *sp.mixOsc;
        const float cut = *sp.cutoff, res = *sp.reso;
        const double sr = getSampleRate();
        const float glideMs = *sp.glide * *sp.glide * 400.0f;
        const double gc = glideMs < 0.1 ? 1.0 : 1.0 - std::exp (-1.0 / (0.001 * glideMs * sr));
        // JP-8000-style detune distribution.
        static const float dpos[7] = { 0.f, -1.f, 1.f, -0.55f, 0.55f, -0.25f, 0.25f };
        fL.set (cut, res); fR.set (cut, res);

        for (int s = 0; s < n; ++s)
        {
            freq += (target - freq) * gc;
            const double f0 = freq * std::pow (2.0, bend / 12.0);
            float l = 0.f, r = 0.f;
            for (int i = 0; i < 7; ++i)
            {
                osc[i].setFrequency (f0 * std::pow (2.0, (double) (dpos[i] * det) / 1200.0));
                const float v = osc[i].next() * (i == 0 ? 1.0f : mix);
                const float pan = dpos[i] * spr;                     // -1..1
                l += v * (0.5f - 0.5f * pan);
                r += v * (0.5f + 0.5f * pan);
            }
            const float e = env.getNextSample() * level * 0.4f;
            out.addSample (0, start + s, fL.process (l, 0) * e);
            if (out.getNumChannels() > 1) out.addSample (1, start + s, fR.process (r, 0) * e);
        }
        if (! env.isActive()) { clearCurrentNote(); fL.reset(); fR.reset(); }
    }

private:
    const SynthParams& sp;
    aur::syn::PolyBlepOsc osc[7];
    aur::syn::SvfZDF fL, fR;
    juce::ADSR env;
    float level = 1.0f;
    double bend = 0.0, freq = 0.0, target = 440.0;
};
