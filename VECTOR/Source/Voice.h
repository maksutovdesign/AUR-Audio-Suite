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
        x=g(ParamID::x); y=g(ParamID::y); orbit=g(ParamID::orbit); rate=g(ParamID::rate);
        cutoff=g(ParamID::cutoff);
        attack=g(ParamID::attack); decay=g(ParamID::decay); sustain=g(ParamID::sustain); release=g(ParamID::release);
    }
    std::atomic<float> *x{},*y{},*orbit{},*rate{},*cutoff{},*attack{},*decay{},*sustain{},*release{};
};

struct VectorSound : juce::SynthesiserSound
{
    bool appliesToNote (int) override { return true; }
    bool appliesToChannel (int) override { return true; }
};

/** Vector synthesis (Prophet-VS/Wavestation idea): 4 corner sources —
    A saw, B pulse, C triangle, D sub sine — mixed by an XY joystick whose
    position can auto-orbit on an ellipse. */
class VectorVoice : public juce::SynthesiserVoice
{
public:
    explicit VectorVoice (const SynthParams& p) : sp (p) {}
    void prepareVoice (double sr)
    {
        setCurrentPlaybackSampleRate (sr);
        for (auto& o : osc) o.prepare (sr);
        filt.prepare (sr);
        env.setSampleRate (sr);
    }
    bool canPlaySound (juce::SynthesiserSound* s) override { return dynamic_cast<VectorSound*> (s) != nullptr; }

    void startNote (int note, float vel, juce::SynthesiserSound*, int wheel) override
    {
        level = 0.25f + 0.75f * vel;
        bend = (wheel - 8192) / 8192.0 * 2.0;
        freq = 440.0 * std::pow (2.0, (note - 69) / 12.0);
        juce::Random r ((juce::int64) note * 2477);
        static const int shapes[4] = { 0, 1, 2, 3 };   // saw pulse tri sine
        for (int i = 0; i < 4; ++i) { osc[i].setShape (shapes[i]); osc[i].reset (r.nextDouble()); }
        orbPhase = 0.0;
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
        const float x0 = *sp.x, y0 = *sp.y, orb = *sp.orbit;
        const double sr = getSampleRate();
        const double oInc = (double) *sp.rate / sr;
        const double f = freq * std::pow (2.0, bend / 12.0);
        for (int i = 0; i < 3; ++i) osc[i].setFrequency (f);
        osc[3].setFrequency (f * 0.5);   // D = sub octave
        filt.set (*sp.cutoff, 0.1f);

        for (int s = 0; s < n; ++s)
        {
            orbPhase += oInc; if (orbPhase >= 1.0) orbPhase -= 1.0;
            const float ox = juce::jlimit (0.0f, 1.0f, x0 + orb * 0.5f * (float) std::sin (6.28318530718 * orbPhase));
            const float oy = juce::jlimit (0.0f, 1.0f, y0 + orb * 0.5f * (float) std::cos (6.28318530718 * orbPhase));

            // Bilinear corner gains (equal-power-ish via sqrt).
            const float gA = std::sqrt ((1.0f - ox) * (1.0f - oy));
            const float gB = std::sqrt (ox * (1.0f - oy));
            const float gC = std::sqrt ((1.0f - ox) * oy);
            const float gD = std::sqrt (ox * oy);

            float v = osc[0].next() * gA + osc[1].next() * gB + osc[2].next() * gC + osc[3].next() * gD;
            v = filt.process (v, 0) * env.getNextSample() * level * 0.8f;
            out.addSample (0, start + s, v);
            if (out.getNumChannels() > 1) out.addSample (1, start + s, v);
        }
        if (! env.isActive()) { clearCurrentNote(); filt.reset(); }
    }

private:
    const SynthParams& sp;
    aur::syn::PolyBlepOsc osc[4];
    aur::syn::SvfZDF filt;
    juce::ADSR env;
    float level = 1.0f;
    double bend = 0.0, freq = 440.0, orbPhase = 0.0;
};
