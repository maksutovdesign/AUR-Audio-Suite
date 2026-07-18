#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <atomic>
#include "Parameters.h"

struct SynthParams
{
    void bind (juce::AudioProcessorValueTreeState& s)
    {
        for (int i = 0; i < 9; ++i) bar[i] = s.getRawParameterValue (ParamID::bar (i));
        perc = s.getRawParameterValue (ParamID::perc);
        vibrato = s.getRawParameterValue (ParamID::vibrato);
        rotary = s.getRawParameterValue (ParamID::rotary);
    }
    std::atomic<float> *bar[9]{}, *perc{}, *vibrato{}, *rotary{};
};

struct OrganSound : juce::SynthesiserSound
{
    bool appliesToNote (int) override { return true; }
    bool appliesToChannel (int) override { return true; }
};

/** Tonewheel organ: 9 sine drawbars at Hammond footages, 2nd/3rd-harmonic
    percussion click, scanner vibrato, simple rotary pan. */
class OrganVoice : public juce::SynthesiserVoice
{
public:
    explicit OrganVoice (const SynthParams& p) : sp (p) {}
    void prepareVoice (double sr) { setCurrentPlaybackSampleRate (sr); }
    bool canPlaySound (juce::SynthesiserSound* s) override { return dynamic_cast<OrganSound*> (s) != nullptr; }

    void startNote (int note, float vel, juce::SynthesiserSound*, int) override
    {
        level = 0.4f + 0.6f * vel;
        freq = 440.0 * std::pow (2.0, (note - 69) / 12.0);
        for (auto& ph : phase) ph = 0.0;
        percEnv = 1.0f; vibPhase = rotPhase = 0.0;
        on = true; relEnv = 1.0f;
    }
    void stopNote (float, bool tail) override { if (tail) on = false; else { relEnv = 0.0f; clearCurrentNote(); } }
    void pitchWheelMoved (int) override {}
    void controllerMoved (int, int) override {}

    void renderNextBlock (juce::AudioBuffer<float>& out, int start, int n) override
    {
        if (relEnv <= 1.0e-4f) return;
        // Hammond drawbar frequency ratios.
        static const double ratio[9] = { 0.5, 1.5, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 8.0 };
        const float perc = *sp.perc, vib = *sp.vibrato;
        const double sr = getSampleRate();
        const float percC = (float) std::exp (-1.0 / (0.2 * sr));
        const double rotHz = 0.5 + *sp.rotary * 6.5;    // slow…fast rotor
        float bars[9]; for (int i = 0; i < 9; ++i) bars[i] = *sp.bar[i];
        const float relC = (float) std::exp (-1.0 / (0.008 * sr));   // organ = fast stop

        for (int s = 0; s < n; ++s)
        {
            vibPhase += 6.4 / sr; if (vibPhase >= 1.0) vibPhase -= 1.0;
            rotPhase += rotHz / sr; if (rotPhase >= 1.0) rotPhase -= 1.0;
            const double fMul = std::pow (2.0, vib * 0.006 * std::sin (6.28318530718 * vibPhase));

            float v = 0.0f;
            for (int i = 0; i < 9; ++i)
            {
                phase[i] += freq * ratio[i] * fMul / sr; if (phase[i] >= 1.0) phase[i] -= 1.0;
                float g = bars[i];
                if (perc > 0.f && (i == 3 || i == 4)) g += perc * percEnv;   // 2nd/3rd click
                if (g > 0.001f) v += (float) std::sin (6.28318530718 * phase[i]) * g;
            }
            percEnv *= percC;
            if (! on) relEnv *= relC;
            v *= 0.18f * level * relEnv;

            const float pan = 0.5f + 0.35f * (float) std::sin (6.28318530718 * rotPhase);
            out.addSample (0, start + s, v * (1.0f - pan) * 2.0f * 0.7f);
            if (out.getNumChannels() > 1) out.addSample (1, start + s, v * pan * 2.0f * 0.7f);
        }
        if (relEnv <= 1.0e-4f) clearCurrentNote();
    }

private:
    const SynthParams& sp;
    double phase[9] { };
    double freq = 440.0, vibPhase = 0.0, rotPhase = 0.0;
    float level = 1.0f, percEnv = 0.0f, relEnv = 1.0f;
    bool on = false;
};
