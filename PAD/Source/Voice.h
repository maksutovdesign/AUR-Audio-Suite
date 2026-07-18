#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <atomic>
#include "Parameters.h"

struct SynthParams
{
    void bind (juce::AudioProcessorValueTreeState& s)
    {
        auto g = [&s](const char* id){ return s.getRawParameterValue (id); };
        bright=g(ParamID::bright); motion=g(ParamID::motion); shimmer=g(ParamID::shimmer); odd=g(ParamID::odd);
        attack=g(ParamID::attack); release=g(ParamID::release);
    }
    std::atomic<float> *bright{},*motion{},*shimmer{},*odd{},*attack{},*release{};
};

struct PadSound : juce::SynthesiserSound
{
    bool appliesToNote (int) override { return true; }
    bool appliesToChannel (int) override { return true; }
};

/** Additive evolving pad: 16 harmonics as detuned stereo sine pairs whose
    amplitudes random-walk slowly (Alchemy-style living spectrum). */
class PadVoice : public juce::SynthesiserVoice
{
public:
    static constexpr int H = 16;
    explicit PadVoice (const SynthParams& p) : sp (p) {}
    void prepareVoice (double sr) { setCurrentPlaybackSampleRate (sr); env.setSampleRate (sr); }
    bool canPlaySound (juce::SynthesiserSound* s) override { return dynamic_cast<PadSound*> (s) != nullptr; }

    void startNote (int note, float vel, juce::SynthesiserSound*, int) override
    {
        level = 0.3f + 0.7f * vel;
        freq = 440.0 * std::pow (2.0, (note - 69) / 12.0);
        rng.setSeed ((juce::int64) note * 15485863);
        for (int h = 0; h < H; ++h)
        {
            phL[h] = rng.nextDouble(); phR[h] = rng.nextDouble();
            walk[h] = rng.nextFloat();
        }
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
        const float bright = *sp.bright, motion = *sp.motion, shim = *sp.shimmer, odd = *sp.odd;
        const double sr = getSampleRate();
        const float wStep = motion * 0.00006f;
        const double nyq = sr * 0.45;

        for (int s = 0; s < n; ++s)
        {
            float l = 0.f, r = 0.f;
            for (int h = 0; h < H; ++h)
            {
                const double fh = freq * (h + 1);
                if (fh > nyq) break;
                // slow bounded random walk per harmonic
                walk[h] += wStep * (rng.nextFloat() * 2.0f - 1.0f);
                walk[h] = juce::jlimit (0.0f, 1.0f, walk[h]);
                // spectral tilt + odd/even balance
                float a = std::pow (0.4f + bright * 0.6f, (float) h) / (1.0f + h * 0.5f);
                a *= (h % 2 == 0) ? (1.0f - odd) + odd : odd + (1.0f - odd) * 0.4f;
                a *= 0.35f + 0.65f * walk[h];

                const double det = 1.0 + (double) shim * 0.0015 * (h + 1) * 0.25;
                phL[h] += fh / det / sr; if (phL[h] >= 1) phL[h] -= 1;
                phR[h] += fh * det / sr; if (phR[h] >= 1) phR[h] -= 1;
                l += (float) std::sin (6.28318530718 * phL[h]) * a;
                r += (float) std::sin (6.28318530718 * phR[h]) * a;
            }
            const float e = env.getNextSample() * level * 0.35f;
            out.addSample (0, start + s, l * e);
            if (out.getNumChannels() > 1) out.addSample (1, start + s, r * e);
        }
        if (! env.isActive()) clearCurrentNote();
    }

private:
    const SynthParams& sp;
    juce::ADSR env;
    juce::Random rng;
    double phL[H] { }, phR[H] { };
    float walk[H] { };
    float level = 1.0f;
    double freq = 440.0;
};
