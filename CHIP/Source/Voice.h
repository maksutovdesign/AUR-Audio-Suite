#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <atomic>
#include "PolyBlepOsc.h"
#include "Parameters.h"

struct SynthParams
{
    void bind (juce::AudioProcessorValueTreeState& s)
    {
        auto g = [&s](const char* id){ return s.getRawParameterValue (id); };
        shape=g(ParamID::shape); crush=g(ParamID::crush); vibrato=g(ParamID::vibrato);
        attack=g(ParamID::attack); decay=g(ParamID::decay); sustain=g(ParamID::sustain); release=g(ParamID::release);
    }
    std::atomic<float> *shape{},*crush{},*vibrato{},*attack{},*decay{},*sustain{},*release{};
};

struct ChipSound : juce::SynthesiserSound
{
    bool appliesToNote (int) override { return true; }
    bool appliesToChannel (int) override { return true; }
};

/** Chiptune voice: NES-style pulse widths / triangle / LFSR noise, 6 Hz
    vibrato, bit-depth crush for the lo-fi character. */
class ChipVoice : public juce::SynthesiserVoice
{
public:
    explicit ChipVoice (const SynthParams& p) : sp (p) {}
    void prepareVoice (double sr) { setCurrentPlaybackSampleRate (sr); osc.prepare (sr); env.setSampleRate (sr); }
    bool canPlaySound (juce::SynthesiserSound* s) override { return dynamic_cast<ChipSound*> (s) != nullptr; }

    void startNote (int note, float vel, juce::SynthesiserSound*, int) override
    {
        level = 0.3f + 0.7f * vel;
        freq = 440.0 * std::pow (2.0, (note - 69) / 12.0);
        osc.reset (0.0); vibPhase = 0.0; lfsr = 0xACE1u;
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
        const int   mode = (int) *sp.shape;
        const float crush = *sp.crush, vib = *sp.vibrato;
        const double sr = getSampleRate();
        const double vibInc = 6.0 / sr;
        // Crush: 0 → 16 levels-ish smooth; 1 → 4 levels (2-bit feel).
        const float steps = 4.0f + (1.0f - crush) * 60.0f;

        for (int s = 0; s < n; ++s)
        {
            vibPhase += vibInc; if (vibPhase >= 1.0) vibPhase -= 1.0;
            const double f = freq * std::pow (2.0, vib * 0.01 * std::sin (6.28318530718 * vibPhase));
            float v;
            if (mode == 4)   // LFSR noise, clocked at pitch*8
            {
                nAcc += f * 8.0 / sr;
                while (nAcc >= 1.0) { nAcc -= 1.0; const unsigned b = ((lfsr >> 0) ^ (lfsr >> 1)) & 1u; lfsr = (lfsr >> 1) | (b << 14); }
                v = (lfsr & 1u) ? 1.0f : -1.0f;
            }
            else
            {
                osc.setFrequency (f);
                if (mode == 3) osc.setShape (2);
                else { osc.setShape (1); osc.setPulseWidth (mode == 0 ? 0.125f : mode == 1 ? 0.25f : 0.5f); }
                v = osc.next();
            }
            v = std::round (v * steps) / steps;          // bit-depth crush
            v *= env.getNextSample() * level;
            out.addSample (0, start + s, v);
            if (out.getNumChannels() > 1) out.addSample (1, start + s, v);
        }
        if (! env.isActive()) clearCurrentNote();
    }

private:
    const SynthParams& sp;
    aur::syn::PolyBlepOsc osc;
    juce::ADSR env;
    float level = 1.0f;
    double freq = 440.0, vibPhase = 0.0, nAcc = 0.0;
    unsigned lfsr = 0xACE1u;
};
