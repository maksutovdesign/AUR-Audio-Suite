#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <atomic>
#include "FMOperator.h"
#include "Parameters.h"

/** Raw-pointer snapshot of NOVA's parameters. */
struct SynthParams
{
    void bind (juce::AudioProcessorValueTreeState& s)
    {
        for (int i = 0; i < 4; ++i)
        {
            ratio[i] = s.getRawParameterValue (ParamID::ratio (i));
            level[i] = s.getRawParameterValue (ParamID::level (i));
            atk[i]   = s.getRawParameterValue (ParamID::atk (i));
            dec[i]   = s.getRawParameterValue (ParamID::dec (i));
            sus[i]   = s.getRawParameterValue (ParamID::sus (i));
            rel[i]   = s.getRawParameterValue (ParamID::rel (i));
        }
        algo = s.getRawParameterValue (ParamID::algo);
        feedback = s.getRawParameterValue (ParamID::feedback);
        glide = s.getRawParameterValue (ParamID::glide);
    }
    std::atomic<float> *ratio[4]{}, *level[4]{}, *atk[4]{}, *dec[4]{}, *sus[4]{}, *rel[4]{},
        *algo{}, *feedback{}, *glide{};
};

struct NovaSound : juce::SynthesiserSound
{
    bool appliesToNote (int) override    { return true; }
    bool appliesToChannel (int) override { return true; }
};

/** 4-operator FM (phase-modulation) voice. Operators are processed high→low so
    modulators (always a higher index than their target) feed carriers within
    the same sample. */
class NovaVoice : public juce::SynthesiserVoice
{
public:
    explicit NovaVoice (const SynthParams& p) : sp (p) {}

    void prepareVoice (double sr)
    {
        setCurrentPlaybackSampleRate (sr);
        for (auto& o : ops) o.prepare (sr);
        for (auto& e : env) e.setSampleRate (sr);
    }

    bool canPlaySound (juce::SynthesiserSound* s) override { return dynamic_cast<NovaSound*> (s) != nullptr; }

    void startNote (int midiNote, float velocity, juce::SynthesiserSound*, int wheel) override
    {
        note = midiNote;
        level = 0.25f + 0.75f * velocity;
        pitchBend = (wheel - 8192) / 8192.0 * 2.0;
        const double target = 440.0 * std::pow (2.0, (midiNote - 69) / 12.0);
        if (currentFreq <= 0.0 || *sp.glide < 1.0e-4f) currentFreq = target;
        targetFreq = target;
        for (auto& o : ops) o.reset (0.0);
        fbState = 0.0f;
        updateEnvParams();
        for (auto& e : env) e.noteOn();
    }

    void stopNote (float, bool allowTailOff) override
    {
        if (allowTailOff) for (auto& e : env) e.noteOff();
        else { for (auto& e : env) e.reset(); clearCurrentNote(); }
    }

    void pitchWheelMoved (int v) override { pitchBend = (v - 8192) / 8192.0 * 2.0; }
    void controllerMoved (int, int) override {}

    void renderNextBlock (juce::AudioBuffer<float>& out, int startSample, int numSamples) override
    {
        if (! env[0].isActive() && ! anyCarrierActive()) return;
        updateEnvParams();

        const int   algo = juce::jlimit (0, 5, (int) *sp.algo);
        const float fb   = *sp.feedback;
        float ratio[4], lvl[4];
        for (int i = 0; i < 4; ++i) { ratio[i] = *sp.ratio[i]; lvl[i] = *sp.level[i]; }

        const auto& edge    = kEdge[algo];      // edge[target] bitmask of sources
        const auto  carrier = kCarrier[algo];
        int nCarBits = 0; for (int b = 0; b < 4; ++b) if (carrier & (1 << b)) ++nCarBits;
        const int   nCar    = juce::jmax (1, nCarBits);
        const float carNorm = 1.0f / std::sqrt ((float) nCar);
        const double sr = getSampleRate();

        const float glideMs   = *sp.glide * *sp.glide * 400.0f;
        const double glideCoef = glideMs < 0.1 ? 1.0 : 1.0 - std::exp (-1.0 / (0.001 * glideMs * sr));

        constexpr float kIndex = 7.0f;   // modulation index scaling (radians)

        for (int s = 0; s < numSamples; ++s)
        {
            currentFreq += (targetFreq - currentFreq) * glideCoef;
            const double f0 = currentFreq * std::pow (2.0, pitchBend / 12.0);

            float sig[4] = { 0, 0, 0, 0 };   // enveloped operator outputs
            for (int op = 3; op >= 0; --op)
            {
                ops[op].setFrequency (f0 * (double) ratio[op]);
                float modIn = 0.0f;
                for (int src = op + 1; src < 4; ++src)
                    if (edge[op] & (1 << src)) modIn += sig[src] * lvl[src] * kIndex;
                if (op == 3 && fb > 0.0f) modIn += fbState * fb * kIndex;

                const float raw = ops[op].render (modIn);
                const float e   = env[op].getNextSample();
                sig[op] = raw * e;
                if (op == 3) fbState = 0.5f * (fbState + sig[op]);   // averaged self-feedback
            }

            float o = 0.0f;
            for (int c = 0; c < 4; ++c)
                if (carrier & (1 << c)) o += sig[c] * lvl[c];
            o *= carNorm * level;

            out.addSample (0, startSample + s, o);
            if (out.getNumChannels() > 1) out.addSample (1, startSample + s, o);
        }

        if (! env[0].isActive() && ! anyCarrierActive())
            clearCurrentNote();
    }

private:
    void updateEnvParams()
    {
        for (int i = 0; i < 4; ++i)
            env[i].setParameters ({ *sp.atk[i], *sp.dec[i], *sp.sus[i], *sp.rel[i] });
    }
    bool anyCarrierActive() const
    {
        const int algo = juce::jlimit (0, 5, (int) *sp.algo);
        for (int c = 0; c < 4; ++c) if ((kCarrier[algo] & (1 << c)) && env[c].isActive()) return true;
        return false;
    }

    // edge[algo][target] = bitmask of source ops modulating target (source idx > target).
    static constexpr int kEdge[6][4] = {
        /* Chain      */ { 0b0010, 0b0100, 0b1000, 0 },
        /* Twin Stack */ { 0b0010, 0,      0b1000, 0 },
        /* 3→Carrier  */ { 0b1110, 0,      0,      0 },
        /* Y-Stack    */ { 0b0110, 0b1000, 0b1000, 0 },
        /* Parallel   */ { 0b0100, 0b1000, 0,      0 },
        /* Additive   */ { 0,      0,      0,      0 },
    };
    static constexpr int kCarrier[6] = {
        0b0001, // Chain      → op1
        0b0101, // Twin Stack → op1, op3
        0b0001, // 3→Carrier  → op1
        0b0001, // Y-Stack    → op1
        0b0011, // Parallel   → op1, op2
        0b1111, // Additive   → all
    };

    const SynthParams& sp;
    aur::syn::FMOperator ops[4];
    juce::ADSR env[4];

    int    note = 60;
    float  level = 1.0f, fbState = 0.0f;
    double pitchBend = 0.0, currentFreq = 0.0, targetFreq = 440.0;
};
