#pragma once

#include <array>
#include "Allpass1.h"
#include "LFO.h"

namespace aur
{
/** Classic allpass phaser — a cascade of first-order allpasses whose break
    frequency is swept by an LFO, with feedback. Control-rate coefficient
    updates keep it cheap. */
class Phaser
{
public:
    static constexpr int kMaxStages = 8;

    void prepare (double sampleRate)
    {
        fs = sampleRate;
        lfo.prepare (fs);
        for (auto& ch : ap) for (auto& a : ch) a.prepare (fs);
        last[0] = last[1] = 0.0f; counter = 0;
    }
    void reset() { for (auto& ch : ap) for (auto& a : ch) a.reset(); last[0] = last[1] = 0.0f; }

    void setParameters (float rateHz, float depthPct, float mixPct, float feedbackPct, int stagesIn)
    {
        lfo.setRate (rateHz);
        depth = depthPct / 100.0f; mix = mixPct / 100.0f;
        fb = std::min (0.95f, feedbackPct / 100.0f);
        stages = stagesIn < 2 ? 2 : (stagesIn > kMaxStages ? kMaxStages : stagesIn);
    }

    void process (float* L, float* R, int numSamples)
    {
        for (int n = 0; n < numSamples; ++n)
        {
            if (--counter <= 0)
            {
                counter = 16;
                const float mL = lfo.at (0.0) * 0.5f + 0.5f;
                const float mR = lfo.at (0.25) * 0.5f + 0.5f;
                const float fL = 200.0f + (200.0f + mL * depth * 3800.0f);
                const float fR = 200.0f + (200.0f + mR * depth * 3800.0f);
                for (int s = 0; s < stages; ++s) { ap[0][(size_t) s].setFrequency (fL); ap[1][(size_t) s].setFrequency (fR); }
            }
            lfo.next();

            float xl = L[n] + fb * last[0];
            float xr = R[n] + fb * last[1];
            for (int s = 0; s < stages; ++s) { xl = ap[0][(size_t) s].process (xl); xr = ap[1][(size_t) s].process (xr); }
            last[0] = xl; last[1] = xr;

            L[n] = L[n] * (1.0f - mix) + xl * mix;
            R[n] = R[n] * (1.0f - mix) + xr * mix;
        }
    }

private:
    double fs = 48000.0;
    std::array<std::array<Allpass1, kMaxStages>, 2> ap;
    LFO lfo;
    float depth = 0.6f, mix = 0.5f, fb = 0.3f, last[2] { 0.0f, 0.0f };
    int stages = 4, counter = 0;
};
} // namespace aur
