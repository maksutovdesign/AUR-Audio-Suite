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
        shape=g(ParamID::shape); pw=g(ParamID::pw); sub=g(ParamID::sub);
        cutoff=g(ParamID::cutoff); reso=g(ParamID::reso); envamt=g(ParamID::envamt); keytrack=g(ParamID::keytrack);
        attack=g(ParamID::attack); decay=g(ParamID::decay); sustain=g(ParamID::sustain); release=g(ParamID::release);
        glide=g(ParamID::glide);
    }
    std::atomic<float> *shape{},*pw{},*sub{},*cutoff{},*reso{},*envamt{},*keytrack{},
        *attack{},*decay{},*sustain{},*release{},*glide{};
};

struct MiniSound : juce::SynthesiserSound
{
    bool appliesToNote (int) override { return true; }
    bool appliesToChannel (int) override { return true; }
};

class MiniVoice : public juce::SynthesiserVoice
{
public:
    explicit MiniVoice (const SynthParams& p) : sp (p) {}

    void prepareVoice (double sr) { setCurrentPlaybackSampleRate (sr); osc.prepare (sr); subOsc.prepare (sr); subOsc.setShape (3); filt.prepare (sr); env.setSampleRate (sr); }
    bool canPlaySound (juce::SynthesiserSound* s) override { return dynamic_cast<MiniSound*> (s) != nullptr; }

    void startNote (int note_, float vel, juce::SynthesiserSound*, int wheel) override
    {
        note = note_; level = 0.2f + 0.8f * vel;
        bend = (wheel - 8192) / 8192.0 * 2.0;
        const double t = 440.0 * std::pow (2.0, (note - 69) / 12.0);
        if (freq <= 0.0 || *sp.glide < 1.0e-4f) freq = t;
        target = t;
        osc.reset (0.0); subOsc.reset (0.0);
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

        const int   sh   = (int) *sp.shape;
        const float pw   = *sp.pw, subLvl = *sp.sub;
        const float base = *sp.cutoff, res = *sp.reso, envAmt = *sp.envamt, kt = *sp.keytrack;
        const double sr  = getSampleRate();
        const float glideMs = *sp.glide * *sp.glide * 400.0f;
        const double gc = glideMs < 0.1 ? 1.0 : 1.0 - std::exp (-1.0 / (0.001 * glideMs * sr));
        const float ktOct = kt * (float) (note - 60) / 12.0f;

        for (int s = 0; s < n; ++s)
        {
            freq += (target - freq) * gc;
            const double f = freq * std::pow (2.0, bend / 12.0);
            osc.setShape (sh); osc.setPulseWidth (pw); osc.setFrequency (f);
            subOsc.setFrequency (f * 0.5);
            const float e = env.getNextSample();

            float v = osc.next() + subOsc.next() * subLvl;
            const float cut = juce::jlimit (20.0f, 20000.0f, base * std::pow (2.0f, envAmt * e * 5.0f + ktOct));
            filt.set (cut, res);
            v = filt.process (v, 0) * e * level;

            out.addSample (0, start + s, v);
            if (out.getNumChannels() > 1) out.addSample (1, start + s, v);
        }
        if (! env.isActive()) { clearCurrentNote(); filt.reset(); }
    }

private:
    const SynthParams& sp;
    aur::syn::PolyBlepOsc osc, subOsc;
    aur::syn::SvfZDF filt;
    juce::ADSR env;
    int note = 60; float level = 1.0f; double bend = 0.0, freq = 0.0, target = 440.0;
};
