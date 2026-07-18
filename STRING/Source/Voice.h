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
        octave=g(ParamID::octave); tone=g(ParamID::tone); attack=g(ParamID::attack); release=g(ParamID::release);
    }
    std::atomic<float> *octave{},*tone{},*attack{},*release{};
};

struct StringSound : juce::SynthesiserSound
{
    bool appliesToNote (int) override { return true; }
    bool appliesToChannel (int) override { return true; }
};

/** Solina-style string voice: saw + slightly-detuned saw + octave saw through
    a gentle LP; the ensemble chorus lives in the processor (shared, like the
    original's BBD ensemble applied to the paraphonic bus). */
class StringVoice : public juce::SynthesiserVoice
{
public:
    explicit StringVoice (const SynthParams& p) : sp (p) {}
    void prepareVoice (double sr)
    {
        setCurrentPlaybackSampleRate (sr);
        for (auto& o : osc) { o.prepare (sr); o.setShape (0); }
        lp.prepare (sr);
        env.setSampleRate (sr);
    }
    bool canPlaySound (juce::SynthesiserSound* s) override { return dynamic_cast<StringSound*> (s) != nullptr; }

    void startNote (int note, float vel, juce::SynthesiserSound*, int) override
    {
        level = 0.3f + 0.7f * vel;
        freq = 440.0 * std::pow (2.0, (note - 69) / 12.0);
        juce::Random r ((juce::int64) note * 31337);
        for (auto& o : osc) o.reset (r.nextDouble());
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
        const float oct = *sp.octave;
        lp.set (*sp.tone, 0.05f);
        osc[0].setFrequency (freq);
        osc[1].setFrequency (freq * 1.004);
        osc[2].setFrequency (freq * 2.0);

        for (int s = 0; s < n; ++s)
        {
            float v = osc[0].next() * 0.5f + osc[1].next() * 0.5f + osc[2].next() * oct * 0.4f;
            v = lp.process (v, 0) * env.getNextSample() * level * 0.6f;
            out.addSample (0, start + s, v);
            if (out.getNumChannels() > 1) out.addSample (1, start + s, v);
        }
        if (! env.isActive()) { clearCurrentNote(); lp.reset(); }
    }

private:
    const SynthParams& sp;
    aur::syn::PolyBlepOsc osc[3];
    aur::syn::SvfZDF lp;
    juce::ADSR env;
    float level = 1.0f;
    double freq = 440.0;
};
