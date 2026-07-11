#pragma once

#include <cmath>

namespace aur
{
/** Bit-depth quantiser + sample-rate decimator (sample-and-hold), dry/wet. */
class BitCrusher
{
public:
    void prepare (double /*fs*/) { reset(); }
    void reset() { for (auto& h : hold) h = 0.0f; for (auto& p : phase) p = 0.0f; }

    /** bits 1..16, downsample 1..50 (1 = none), mix 0..100. */
    void setParameters (float bits, float downsample, float mixPercent)
    {
        levels = std::pow (2.0f, bits) - 1.0f;
        step   = downsample < 1.0f ? 1.0f : downsample;
        mix    = mixPercent / 100.0f;
    }

    void process (float* const* data, int numCh, int numSamples)
    {
        for (int ch = 0; ch < numCh; ++ch)
        {
            const size_t c = (size_t) (ch < (int) kMaxCh ? ch : kMaxCh - 1);
            auto* d = data[ch];
            for (int n = 0; n < numSamples; ++n)
            {
                phase[c] += 1.0f;
                if (phase[c] >= step)
                {
                    phase[c] -= step;
                    const float q = std::round (d[n] * levels) / levels; // bit reduce
                    hold[c] = q;
                }
                d[n] = d[n] * (1.0f - mix) + hold[c] * mix;
            }
        }
    }

private:
    static constexpr size_t kMaxCh = 2;
    float hold[kMaxCh] { 0.0f, 0.0f }, phase[kMaxCh] { 0.0f, 0.0f };
    float levels = 65535.0f, step = 1.0f, mix = 1.0f;
};
} // namespace aur
