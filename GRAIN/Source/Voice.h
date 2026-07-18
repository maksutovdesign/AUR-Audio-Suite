#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <atomic>
#include "WavetableOsc.h"
#include "Parameters.h"

struct SynthParams
{
    void bind (juce::AudioProcessorValueTreeState& s)
    {
        auto g = [&s](const char* id){ return s.getRawParameterValue (id); };
        density=g(ParamID::density); size=g(ParamID::size); spray=g(ParamID::spray); texture=g(ParamID::texture);
        attack=g(ParamID::attack); release=g(ParamID::release);
    }
    std::atomic<float> *density{},*size{},*spray{},*texture{},*attack{},*release{};
};

struct GrainSound : juce::SynthesiserSound
{
    bool appliesToNote (int) override { return true; }
    bool appliesToChannel (int) override { return true; }
};

/** Granular cloud voice: up to 12 overlapping Hann-windowed grains, each a
    wavetable osc at note pitch × random spray, random pan, Poisson-ish spawn. */
class GrainVoice : public juce::SynthesiserVoice
{
public:
    static constexpr int MAXG = 12;
    explicit GrainVoice (const SynthParams& p) : sp (p) {}
    void prepareVoice (double sr)
    {
        setCurrentPlaybackSampleRate (sr);
        for (auto& g : grains) g.osc.prepare (sr);
        env.setSampleRate (sr);
    }
    bool canPlaySound (juce::SynthesiserSound* s) override { return dynamic_cast<GrainSound*> (s) != nullptr; }

    void startNote (int note, float vel, juce::SynthesiserSound*, int) override
    {
        level = 0.3f + 0.7f * vel;
        freq = 440.0 * std::pow (2.0, (note - 69) / 12.0);
        for (auto& g : grains) g.age = -1;
        spawnTimer = 0;
        rng.setSeed ((juce::int64) note * 48271);
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
        const double sr = getSampleRate();
        const int   glen = juce::jmax (16, (int) ((double) *sp.size * sr));
        const int   interval = juce::jmax (8, (int) (sr / (double) *sp.density));
        const float spray = *sp.spray * 700.0f;   // cents
        const float tex = *sp.texture;

        for (int s = 0; s < n; ++s)
        {
            if (--spawnTimer <= 0)
            {
                spawnTimer = interval;
                for (auto& g : grains)
                    if (g.age < 0)
                    {
                        g.age = 0; g.len = glen;
                        g.osc.reset (rng.nextDouble());
                        g.osc.setMorph (juce::jlimit (0.0f, 1.0f, tex + 0.15f * (rng.nextFloat() * 2 - 1)));
                        g.osc.setFrequency (freq * std::pow (2.0, (double) (spray * (rng.nextFloat() * 2 - 1)) / 1200.0));
                        g.pan = rng.nextFloat();
                        break;
                    }
            }

            float l = 0.f, r = 0.f;
            for (auto& g : grains)
            {
                if (g.age < 0) continue;
                const float w = 0.5f * (1.0f - (float) std::cos (6.28318530718 * g.age / (double) g.len));  // Hann
                const float v = g.osc.next() * w;
                l += v * (1.0f - g.pan); r += v * g.pan;
                if (++g.age >= g.len) g.age = -1;
            }
            const float e = env.getNextSample() * level * 0.5f;
            out.addSample (0, start + s, l * e);
            if (out.getNumChannels() > 1) out.addSample (1, start + s, r * e);
        }
        if (! env.isActive()) clearCurrentNote();
    }

private:
    struct Grain { aur::syn::WavetableOsc osc; int age = -1, len = 0; float pan = 0.5f; };
    const SynthParams& sp;
    Grain grains[MAXG];
    juce::ADSR env;
    juce::Random rng;
    float level = 1.0f;
    double freq = 440.0;
    int spawnTimer = 0;
};
