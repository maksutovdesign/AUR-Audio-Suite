#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <atomic>
#include "Parameters.h"

struct SynthParams
{
    void bind (juce::AudioProcessorValueTreeState& s)
    {
        auto g = [&s](const char* id){ return s.getRawParameterValue (id); };
        punch=g(ParamID::punch); harm=g(ParamID::harm);
        attack=g(ParamID::attack); decay=g(ParamID::decay); sustain=g(ParamID::sustain); release=g(ParamID::release);
        glide=g(ParamID::glide);
    }
    std::atomic<float> *punch{},*harm{},*attack{},*decay{},*sustain{},*release{},*glide{};
};

struct SubSound : juce::SynthesiserSound
{
    bool appliesToNote (int) override { return true; }
    bool appliesToChannel (int) override { return true; }
};

/** Pure sub bass: sine + dosable 2nd harmonic + pitch "punch" transient. */
class SubVoice : public juce::SynthesiserVoice
{
public:
    explicit SubVoice (const SynthParams& p) : sp (p) {}
    void prepareVoice (double sr) { setCurrentPlaybackSampleRate (sr); env.setSampleRate (sr); }
    bool canPlaySound (juce::SynthesiserSound* s) override { return dynamic_cast<SubSound*> (s) != nullptr; }

    void startNote (int note, float vel, juce::SynthesiserSound*, int wheel) override
    {
        level = 0.3f + 0.7f * vel;
        bend = (wheel - 8192) / 8192.0 * 2.0;
        const double t = 440.0 * std::pow (2.0, (note - 69) / 12.0);
        if (freq <= 0.0 || *sp.glide < 1.0e-4f) freq = t;
        target = t; phase = 0.0; pEnv = 1.0f;
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
        const float punch = *sp.punch, harm = *sp.harm;
        const double sr = getSampleRate();
        const float glideMs = *sp.glide * *sp.glide * 300.0f;
        const double gc = glideMs < 0.1 ? 1.0 : 1.0 - std::exp (-1.0 / (0.001 * glideMs * sr));
        const float pC = (float) std::exp (-1.0 / (0.015 * sr));   // ~15 ms punch

        for (int s = 0; s < n; ++s)
        {
            freq += (target - freq) * gc;
            const double f = freq * std::pow (2.0, bend / 12.0) * (1.0 + 2.0 * punch * pEnv);
            pEnv *= pC;
            phase += f / sr; if (phase >= 1.0) phase -= 1.0;
            const float e = env.getNextSample();
            const float v = ((float) std::sin (6.283185307179586 * phase)
                          + harm * (float) std::sin (12.566370614359172 * phase) * 0.5f) * e * level;
            out.addSample (0, start + s, v);
            if (out.getNumChannels() > 1) out.addSample (1, start + s, v);
        }
        if (! env.isActive()) clearCurrentNote();
    }

private:
    const SynthParams& sp;
    juce::ADSR env;
    float level = 1.0f, pEnv = 0.0f;
    double bend = 0.0, freq = 0.0, target = 440.0, phase = 0.0;
};

using SubVoiceT = SubVoice; // scaffold compatibility
