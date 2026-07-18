#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <atomic>
#include <vector>
#include "SvfZDF.h"
#include "Parameters.h"

struct SynthParams
{
    void bind (juce::AudioProcessorValueTreeState& s)
    {
        auto g = [&s](const char* id){ return s.getRawParameterValue (id); };
        exciter=g(ParamID::exciter); pick=g(ParamID::pick); resfb=g(ParamID::resfb);
        damp=g(ParamID::damp); body=g(ParamID::body);
    }
    std::atomic<float> *exciter{},*pick{},*resfb{},*damp{},*body{};
};

struct ResSound : juce::SynthesiserSound
{
    bool appliesToNote (int) override { return true; }
    bool appliesToChannel (int) override { return true; }
};

/** Resonator: noise exciter (burst + sustained breath) driving a tuned comb
    (waveguide loop with one-pole damping) plus a bandpass "body" at f0.
    Bowed/blown textures at high exciter, plucked at high pick. */
class ResVoice : public juce::SynthesiserVoice
{
public:
    explicit ResVoice (const SynthParams& p) : sp (p) {}
    void prepareVoice (double sr)
    {
        setCurrentPlaybackSampleRate (sr);
        buf.assign ((size_t) (sr / 25.0) + 8, 0.0f);
        bp.prepare (sr);
        env.setSampleRate (sr);
    }
    bool canPlaySound (juce::SynthesiserSound* s) override { return dynamic_cast<ResSound*> (s) != nullptr; }

    void startNote (int note, float vel, juce::SynthesiserSound*, int) override
    {
        level = 0.3f + 0.7f * vel;
        const double f = 440.0 * std::pow (2.0, (note - 69) / 12.0);
        size = juce::jlimit (4, (int) buf.size() - 1, (int) std::round (getSampleRate() / f));
        std::fill (buf.begin(), buf.end(), 0.0f);
        idx = 0; lp = 0.0f;
        f0 = (float) f;
        burst = *sp.pick * level;
        rng = 0x77aa11u ^ (unsigned) note;
        env.setParameters ({ 0.005f, 0.1f, 1.0f, 0.4f });
        env.noteOn();
    }
    void stopNote (float, bool tail) override { if (tail) env.noteOff(); else { env.reset(); clearCurrentNote(); } }
    void pitchWheelMoved (int) override {}
    void controllerMoved (int, int) override {}

    void renderNextBlock (juce::AudioBuffer<float>& out, int start, int n) override
    {
        if (! env.isActive() && ringEnv < 1.0e-5f) return;
        const float exc = *sp.exciter, fb = 0.85f + *sp.resfb * 0.1499f;
        const float dampC = 0.15f + *sp.damp * 0.8f;
        const float body = *sp.body;
        bp.set (f0, 0.6f);
        const double sr = getSampleRate();
        const float bC = (float) std::exp (-1.0 / (0.005 * sr));

        for (int s = 0; s < n; ++s)
        {
            const float e = env.getNextSample();
            rng = rng * 1664525u + 1013904223u;
            const float nz = ((float) (rng >> 8) / 8388608.0f - 1.0f);
            float in = nz * (burst + exc * 0.25f * e * level);
            burst *= bC;

            const int nxt = (idx + 1) % size;
            const float cur = buf[(size_t) idx];
            lp += dampC * (0.5f * (cur + buf[(size_t) nxt]) - lp);
            buf[(size_t) idx] = in + lp * fb;
            idx = nxt;

            float v = cur + body * bp.process (cur, 1) * 2.0f;
            v *= 0.7f;
            ringEnv = 0.999f * ringEnv + 0.001f * std::abs (v);
            out.addSample (0, start + s, v);
            if (out.getNumChannels() > 1) out.addSample (1, start + s, v);
        }
        if (! env.isActive() && ringEnv < 1.0e-5f) clearCurrentNote();
    }

private:
    const SynthParams& sp;
    std::vector<float> buf;
    aur::syn::SvfZDF bp;
    juce::ADSR env;
    int size = 100, idx = 0;
    float lp = 0.f, f0 = 440.f, burst = 0.f, level = 1.f, ringEnv = 0.f;
    unsigned rng = 1;
};
