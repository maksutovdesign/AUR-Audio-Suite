#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <atomic>
#include "Parameters.h"

struct SynthParams
{
    void bind (juce::AudioProcessorValueTreeState& s)
    {
        auto g = [&s](const char* id){ return s.getRawParameterValue (id); };
        material=g(ParamID::material); decay=g(ParamID::decay); bright=g(ParamID::bright);
        strike=g(ParamID::strike); inharm=g(ParamID::inharm);
    }
    std::atomic<float> *material{},*decay{},*bright{},*strike{},*inharm{};
};

struct BellSound : juce::SynthesiserSound
{
    bool appliesToNote (int) override { return true; }
    bool appliesToChannel (int) override { return true; }
};

/** Modal synthesis: a bank of 8 exponentially-decaying sine partials whose
    ratios come from measured material tables (bell/bar/glass/membrane).
    Higher partials decay faster (physically correct). */
class BellVoice : public juce::SynthesiserVoice
{
public:
    explicit BellVoice (const SynthParams& p) : sp (p) {}
    void prepareVoice (double sr) { setCurrentPlaybackSampleRate (sr); }
    bool canPlaySound (juce::SynthesiserSound* s) override { return dynamic_cast<BellSound*> (s) != nullptr; }

    void startNote (int note, float vel, juce::SynthesiserSound*, int) override
    {
        static const double tabs[4][8] = {
            { 0.56, 0.92, 1.19, 1.71, 2.00, 2.74, 3.00, 3.76 },  // church bell (Rossing)
            { 1.00, 2.76, 5.40, 8.93, 13.34, 18.64, 24.8, 31.9 }, // bar/marimba
            { 1.00, 2.32, 4.25, 6.63, 9.38, 12.5, 16.0, 19.9 },   // glass/wine
            { 1.00, 1.59, 2.14, 2.30, 2.65, 2.92, 3.16, 3.50 },   // membrane/drum
        };
        const int m = juce::jlimit (0, 3, (int) *sp.material);
        const float stretch = 1.0f + *sp.inharm * 0.06f;
        const double f0 = 440.0 * std::pow (2.0, (note - 69) / 12.0);
        const double sr = getSampleRate();
        const float dec = 0.3f + *sp.decay * *sp.decay * 12.0f;   // seconds for partial 1
        const float tilt = *sp.bright;

        for (int i = 0; i < 8; ++i)
        {
            double r = tabs[m][i];
            r = std::pow (r, (double) stretch);
            const double f = f0 * (m == 0 ? r / 0.56 : r);        // bell table normalised to prime
            act[i] = f < sr * 0.45;
            phase[i] = 0.0;
            inc[i] = f / sr;
            // Amplitude tilt: bright lifts the top, dark rolls it off.
            amp[i] = (float) std::pow (0.5 + tilt, (double) i * 0.7) / (1.0f + (float) i * 0.7f);
            // Higher modes die faster: t60 ∝ 1/(1+i).
            coef[i] = (float) std::exp (-6.9077553 / (dec / (1.0 + 0.8 * i) * sr));
            env[i] = (0.35f + 0.65f * vel);
        }
        strikeEnv = *sp.strike * vel; rng = 0x1234567u ^ (unsigned) note;
    }
    void stopNote (float, bool tail) override { if (! tail) { for (auto& e : env) e = 0.f; clearCurrentNote(); } }
    void pitchWheelMoved (int) override {}
    void controllerMoved (int, int) override {}

    void renderNextBlock (juce::AudioBuffer<float>& out, int start, int n) override
    {
        float total = 0.f; for (int i = 0; i < 8; ++i) total += env[i];
        if (total < 1.0e-5f && strikeEnv < 1.0e-5f) { clearCurrentNote(); return; }
        const double sr = getSampleRate();
        const float sC = (float) std::exp (-1.0 / (0.004 * sr));

        for (int s = 0; s < n; ++s)
        {
            float v = 0.0f;
            for (int i = 0; i < 8; ++i)
            {
                if (! act[i] || env[i] < 1.0e-6f) continue;
                phase[i] += inc[i]; if (phase[i] >= 1.0) phase[i] -= 1.0;
                v += (float) std::sin (6.28318530718 * phase[i]) * amp[i] * env[i];
                env[i] *= coef[i];
            }
            if (strikeEnv > 1.0e-6f)
            {
                rng = rng * 1664525u + 1013904223u;
                v += ((float) (rng >> 8) / 8388608.0f - 1.0f) * strikeEnv * 0.5f;
                strikeEnv *= sC;
            }
            v *= 0.5f;
            out.addSample (0, start + s, v);
            if (out.getNumChannels() > 1) out.addSample (1, start + s, v);
        }
    }

private:
    const SynthParams& sp;
    double phase[8] { }, inc[8] { };
    float amp[8] { }, env[8] { }, coef[8] { }, strikeEnv = 0.f;
    bool act[8] { };
    unsigned rng = 1;
};
