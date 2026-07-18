#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <atomic>
#include "PolyBlepOsc.h"
#include "WavetableOsc.h"
#include "SvfZDF.h"
#include "Parameters.h"

struct SynthParams
{
    void bind (juce::AudioProcessorValueTreeState& s)
    {
        auto g = [&s](const char* id){ return s.getRawParameterValue (id); };
        morph=g(ParamID::morph); motion=g(ParamID::motion); rate=g(ParamID::rate);
        cutoff=g(ParamID::cutoff); reso=g(ParamID::reso);
        attack=g(ParamID::attack); decay=g(ParamID::decay); sustain=g(ParamID::sustain); release=g(ParamID::release);
    }
    std::atomic<float> *morph{},*motion{},*rate{},*cutoff{},*reso{},*attack{},*decay{},*sustain{},*release{};
};

struct MorphSound : juce::SynthesiserSound
{
    bool appliesToNote (int) override { return true; }
    bool appliesToChannel (int) override { return true; }
};

/** Engine morpher: one macro knob glides through 4 different synthesis
    engines — VA saw stack → FM pair → wavetable → additive organ — always
    crossfading the two adjacent ones. Motion LFO sweeps the macro. */
class MorphVoice : public juce::SynthesiserVoice
{
public:
    explicit MorphVoice (const SynthParams& p) : sp (p) {}
    void prepareVoice (double sr)
    {
        setCurrentPlaybackSampleRate (sr);
        va1.prepare (sr); va2.prepare (sr); va1.setShape (0); va2.setShape (0);
        wt.prepare (sr);
        filt.prepare (sr);
        env.setSampleRate (sr);
    }
    bool canPlaySound (juce::SynthesiserSound* s) override { return dynamic_cast<MorphSound*> (s) != nullptr; }

    void startNote (int note, float vel, juce::SynthesiserSound*, int) override
    {
        level = 0.25f + 0.75f * vel;
        freq = 440.0 * std::pow (2.0, (note - 69) / 12.0);
        juce::Random r ((juce::int64) note * 3571);
        va1.reset (r.nextDouble()); va2.reset (r.nextDouble());
        wt.reset (r.nextDouble()); wt.setMorph (0.66f);
        pc = pm = 0.0; for (auto& ph : add) ph = r.nextDouble();
        lfoPhase = 0.0;
        env.setParameters ({ *sp.attack, *sp.decay, *sp.sustain, *sp.release });
        env.noteOn();
    }
    void stopNote (float, bool tail) override { if (tail) env.noteOff(); else { env.reset(); clearCurrentNote(); } }
    void pitchWheelMoved (int) override {}
    void controllerMoved (int, int) override {}

    void renderNextBlock (juce::AudioBuffer<float>& out, int start, int n) override
    {
        if (! env.isActive()) return;
        env.setParameters ({ *sp.attack, *sp.decay, *sp.sustain, *sp.release });
        const float mo0 = *sp.morph, mot = *sp.motion;
        const double sr = getSampleRate();
        const double lfoInc = (double) *sp.rate / sr;
        filt.set (*sp.cutoff, *sp.reso);
        va1.setFrequency (freq * 0.9967); va2.setFrequency (freq * 1.0033);
        wt.setFrequency (freq);

        for (int s = 0; s < n; ++s)
        {
            lfoPhase += lfoInc; if (lfoPhase >= 1.0) lfoPhase -= 1.0;
            const float m = juce::jlimit (0.0f, 1.0f,
                mo0 + mot * 0.4f * (float) std::sin (6.28318530718 * lfoPhase)) * 3.0f;
            const int e0 = juce::jlimit (0, 3, (int) m);
            const int e1 = juce::jmin (e0 + 1, 3);
            const float fr = m - (float) e0;

            auto engine = [&] (int idx) -> float
            {
                switch (idx)
                {
                    case 0: return (va1.next() + va2.next()) * 0.5f;                       // VA
                    case 1:                                                                 // FM pair
                    {
                        pm += freq * 2.0 / sr; if (pm >= 1) pm -= 1;
                        pc += freq / sr; if (pc >= 1) pc -= 1;
                        return (float) std::sin (6.28318530718 * pc + 3.0 * std::sin (6.28318530718 * pm));
                    }
                    case 2: return wt.next();                                              // wavetable
                    default:                                                                // additive organ
                    {
                        float v = 0.f;
                        static const double rr[4] = { 1.0, 2.0, 3.0, 4.0 };
                        static const float aa[4] = { 0.6f, 0.35f, 0.25f, 0.15f };
                        for (int h = 0; h < 4; ++h)
                        {
                            add[h] += freq * rr[h] / sr; if (add[h] >= 1) add[h] -= 1;
                            v += (float) std::sin (6.28318530718 * add[h]) * aa[h];
                        }
                        return v;
                    }
                }
            };
            // NB: both engines advance their own phase only when computed; adjacent
            // pair is always computed so the crossfade stays click-free.
            const float vA = engine (e0);
            const float vB = e1 != e0 ? engine (e1) : vA;
            float v = vA * (1.0f - fr) + vB * fr;
            v = filt.process (v, 0) * env.getNextSample() * level * 0.8f;
            out.addSample (0, start + s, v);
            if (out.getNumChannels() > 1) out.addSample (1, start + s, v);
        }
        if (! env.isActive()) { clearCurrentNote(); filt.reset(); }
    }

private:
    const SynthParams& sp;
    aur::syn::PolyBlepOsc va1, va2;
    aur::syn::WavetableOsc wt;
    aur::syn::SvfZDF filt;
    juce::ADSR env;
    double pc = 0, pm = 0, add[4] { };
    float level = 1.0f;
    double freq = 440.0, lfoPhase = 0.0;
};
