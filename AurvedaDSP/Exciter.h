#pragma once

#include <array>
#include <juce_dsp/juce_dsp.h>
#include "Biquad.h"
#include "ADAASaturator.h"

namespace aur
{
/**
    Harmonic exciter. High-passes the signal, runs the high band through the
    ADAA saturator to generate clean (alias-suppressed) harmonics, then adds
    them back — adds "air"/sheen without muddying the body. Realtime-safe.
*/
class Exciter
{
public:
    void prepare (double sampleRate, int /*ch*/, int maxBlock)
    {
        fs = sampleRate;
        sat.prepare (fs);
        scratch.setSize (2, maxBlock);
        for (auto& f : hp) f.setHighpass (fs, 3000.0, 0.707);
    }
    void reset() { sat.reset(); for (auto& f : hp) f.reset(); }

    void setParameters (float freqHz, float amountPct, float mixPct)
    {
        for (auto& f : hp) f.setHighpass (fs, freqHz, 0.707);
        sat.setParameters (amountPct, 100.0f);
        mix = mixPct / 100.0f;
    }

    void process (float* const* data, int numCh, int numSamples)
    {
        const int nc = numCh < 2 ? numCh : 2;
        for (int c = 0; c < nc; ++c)
        {
            auto* s = scratch.getWritePointer (c);
            for (int i = 0; i < numSamples; ++i) s[i] = hp[(size_t) c].process (data[c][i]);
        }

        juce::dsp::AudioBlock<float> block (scratch);
        auto sub = block.getSubsetChannelBlock (0, (size_t) nc).getSubBlock (0, (size_t) numSamples);
        sat.process (sub);

        for (int c = 0; c < nc; ++c)
        {
            const auto* s = scratch.getReadPointer (c);
            for (int i = 0; i < numSamples; ++i) data[c][i] += mix * s[i];
        }
    }

private:
    double fs = 48000.0;
    std::array<Biquad, 2> hp;
    ADAASaturator sat;
    juce::AudioBuffer<float> scratch;
    float mix = 0.5f;
};
} // namespace aur
