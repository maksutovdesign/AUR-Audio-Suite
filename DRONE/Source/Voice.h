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
        detune=g(ParamID::detune); motion=g(ParamID::motion); cutoff=g(ParamID::cutoff); reso=g(ParamID::reso);
        attack=g(ParamID::attack); release=g(ParamID::release);
    }
    std::atomic<float> *detune{},*motion{},*cutoff{},*reso{},*attack{},*release{};
};

struct DroneSound : juce::SynthesiserSound
{
    bool appliesToNote (int) override { return true; }
    bool appliesToChannel (int) override { return true; }
};

/** Drone generator: 4 slowly-drifting detuned saws + sub sine through a slowly
    breathing SVF. Two incommensurate LFOs (0.07/0.11 Hz) keep it ever-moving. */
class DroneVoice : public juce::SynthesiserVoice
{
public:
    explicit DroneVoice (const SynthParams& p) : sp (p) {}
    void prepareVoice (double sr)
    {
        setCurrentPlaybackSampleRate (sr);
        for (auto& o : osc) o.prepare (sr);
        sub.prepare (sr); sub.setShape (3);
        fL.prepare (sr); fR.prepare (sr);
        env.setSampleRate (sr);
    }
    bool canPlaySound (juce::SynthesiserSound* s) override { return dynamic_cast<DroneSound*> (s) != nullptr; }

    void startNote (int note, float vel, juce::SynthesiserSound*, int) override
    {
        level = 0.3f + 0.7f * vel;
        freq = 440.0 * std::pow (2.0, (note - 69) / 12.0);
        juce::Random r ((juce::int64) note * 104729);
        for (auto& o : osc) { o.setShape (0); o.reset (r.nextDouble()); }
        sub.reset (0.0);
        env.setParameters ({ *sp.attack, 0.1f, 1.0f, *sp.release });
        env.noteOn();
    }
    void stopNote (float, bool tail) override { if (tail) env.noteOff(); else { env.reset(); clearCurrentNote(); } }
    void pitchWheelMoved (int) override {}
    void controllerMoved (int, int) override {}

    void renderNextBlock (juce::AudioBuffer<float>& out, int start, int n) override
    {
        if (! env.isActive()) return;
        env.setParameters ({ *sp.attack, 0.1f, 1.0f, *sp.release });
        const float det = *sp.detune * 25.0f, mot = *sp.motion;
        const float baseCut = *sp.cutoff, res = *sp.reso;
        const double sr = getSampleRate();
        const double i1 = 0.07 / sr, i2 = 0.11 / sr;
        static const float dpos[4] = { -1.f, -0.33f, 0.33f, 1.f };

        for (int s = 0; s < n; ++s)
        {
            p1 += i1; if (p1 >= 1.0) p1 -= 1.0;
            p2 += i2; if (p2 >= 1.0) p2 -= 1.0;
            const float l1 = (float) std::sin (6.28318530718 * p1);
            const float l2 = (float) std::sin (6.28318530718 * p2);

            float l = 0.f, r = 0.f;
            for (int i = 0; i < 4; ++i)
            {
                const float drift = det * (dpos[i] + mot * 0.4f * (i % 2 ? l1 : l2));
                osc[i].setFrequency (freq * std::pow (2.0, (double) drift / 1200.0));
                const float v = osc[i].next() * 0.3f;
                l += v * (0.5f - 0.4f * dpos[i]);
                r += v * (0.5f + 0.4f * dpos[i]);
            }
            sub.setFrequency (freq * 0.5);
            const float sb = sub.next() * 0.4f; l += sb; r += sb;

            const float cut = juce::jlimit (40.0f, 18000.0f, baseCut * std::pow (2.0f, mot * 0.8f * l2));
            if ((ctrl++ & 31) == 0) { fL.set (cut, res); fR.set (cut, res); }
            const float e = env.getNextSample() * level;
            out.addSample (0, start + s, fL.process (l, 0) * e);
            if (out.getNumChannels() > 1) out.addSample (1, start + s, fR.process (r, 0) * e);
        }
        if (! env.isActive()) { clearCurrentNote(); fL.reset(); fR.reset(); }
    }

private:
    const SynthParams& sp;
    aur::syn::PolyBlepOsc osc[4], sub;
    aur::syn::SvfZDF fL, fR;
    juce::ADSR env;
    float level = 1.0f;
    double freq = 110.0, p1 = 0.0, p2 = 0.25;
    unsigned ctrl = 0;
};
