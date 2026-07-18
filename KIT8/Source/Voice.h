#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <atomic>
#include "SvfZDF.h"
#include "Parameters.h"

struct SynthParams
{
    void bind (juce::AudioProcessorValueTreeState& s)
    {
        auto g = [&s](const char* id){ return s.getRawParameterValue (id); };
        sweep=g(ParamID::sweep); punch=g(ParamID::punch); decay=g(ParamID::decay);
        tone=g(ParamID::tone); glide=g(ParamID::glide);
    }
    std::atomic<float> *sweep{},*punch{},*decay{},*tone{},*glide{};
};

struct Kit8Sound : juce::SynthesiserSound
{
    bool appliesToNote (int) override { return true; }
    bool appliesToChannel (int) override { return true; }
};

/** Melodic 808: sine with an exponential pitch drop (sweep), click transient,
    long AD tail — the modern pitched 808 bass. Mono-friendly with glide. */
class Kit8Voice : public juce::SynthesiserVoice
{
public:
    explicit Kit8Voice (const SynthParams& p) : sp (p) {}
    void prepareVoice (double sr) { setCurrentPlaybackSampleRate (sr); lp.prepare (sr); }
    bool canPlaySound (juce::SynthesiserSound* s) override { return dynamic_cast<Kit8Sound*> (s) != nullptr; }

    void startNote (int note, float vel, juce::SynthesiserSound*, int) override
    {
        level = 0.4f + 0.6f * vel;
        const double t = 440.0 * std::pow (2.0, (note - 69) / 12.0);
        if (freq <= 0.0 || *sp.glide < 1.0e-4f) freq = t;
        target = t;
        pEnv = 1.0f; amp = 1.0f; click = *sp.punch * level;
        const double sr = getSampleRate();
        ampC = (float) std::exp (-1.0 / ((double) *sp.decay * sr));
        pC   = (float) std::exp (-1.0 / (0.03 * sr));
        released = false; relC = 1.0f;
    }
    void stopNote (float, bool tail) override
    {
        if (tail) { released = true; relC = (float) std::exp (-1.0 / (0.08 * getSampleRate())); }
        else { amp = 0.f; clearCurrentNote(); }
    }
    void pitchWheelMoved (int) override {}
    void controllerMoved (int, int) override {}

    void renderNextBlock (juce::AudioBuffer<float>& out, int start, int n) override
    {
        if (amp < 1.0e-5f) return;
        const float sweep = *sp.sweep;
        const double sr = getSampleRate();
        const float glideMs = *sp.glide * *sp.glide * 250.0f;
        const double gc = glideMs < 0.1 ? 1.0 : 1.0 - std::exp (-1.0 / (0.001 * glideMs * sr));
        lp.set (*sp.tone, 0.1f);

        for (int s = 0; s < n; ++s)
        {
            freq += (target - freq) * gc;
            const double f = freq * (1.0 + 5.0 * sweep * pEnv);
            pEnv *= pC;
            phase += f / sr; if (phase >= 1.0) phase -= 1.0;
            float v = (float) std::sin (6.28318530718 * phase) * amp;
            v += click; click *= 0.4f;
            amp *= ampC;
            if (released) amp *= relC;
            v = lp.process (v, 0) * level;
            out.addSample (0, start + s, v);
            if (out.getNumChannels() > 1) out.addSample (1, start + s, v);
        }
        if (amp < 1.0e-5f) { clearCurrentNote(); lp.reset(); }
    }

private:
    const SynthParams& sp;
    aur::syn::SvfZDF lp;
    float level = 1.f, amp = 0.f, ampC = 0.f, pEnv = 0.f, pC = 0.f, click = 0.f, relC = 1.f;
    bool released = false;
    double freq = 0.0, target = 55.0, phase = 0.0;
};
