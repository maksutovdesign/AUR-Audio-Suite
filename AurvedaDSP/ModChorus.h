#pragma once

#include <array>
#include "FractionalDelay.h"
#include "LFO.h"

namespace aur
{
/** Modulated delay — powers both chorus (longer delay) and flanger (short
    delay + feedback). Stereo via a quarter-cycle LFO offset. */
class ModChorus
{
public:
    void prepare (double sampleRate, double baseMs, double maxMs)
    {
        fs = sampleRate; base = baseMs;
        dl[0].prepare (fs, maxMs); dl[1].prepare (fs, maxMs);
        lfo.prepare (fs);
        last[0] = last[1] = 0.0f;
    }
    void reset() { dl[0].reset(); dl[1].reset(); last[0] = last[1] = 0.0f; }

    void setParameters (float rateHz, float depthMs, float mixPct, float feedbackPct)
    {
        lfo.setRate (rateHz);
        depth = depthMs; mix = mixPct / 100.0f;
        fb = std::min (0.95f, feedbackPct / 100.0f);
    }

    void process (float* L, float* R, int numSamples)
    {
        for (int n = 0; n < numSamples; ++n)
        {
            // Unipolar modulation keeps the delay in [base, base+depth] — never
            // negative — which is what a flanger/chorus needs (bipolar swing with
            // depth>base would read past the write head and squeal).
            const float m  = lfo.next()   * 0.5f + 0.5f;
            const float mR = lfo.at (0.25) * 0.5f + 0.5f;
            const float dL = (float) ((base + depth * m)  * 0.001 * fs);
            const float dR = (float) ((base + depth * mR) * 0.001 * fs);

            dl[0].write (L[n] + fb * last[0]);
            dl[1].write (R[n] + fb * last[1]);
            const float wl = dl[0].read (dL);
            const float wr = dl[1].read (dR);
            last[0] = wl; last[1] = wr;

            L[n] = L[n] * (1.0f - mix) + wl * mix;
            R[n] = R[n] * (1.0f - mix) + wr * mix;
        }
    }

private:
    double fs = 48000.0, base = 18.0;
    std::array<FractionalDelay, 2> dl;
    LFO lfo;
    float depth = 5.0f, mix = 0.5f, fb = 0.0f, last[2] { 0.0f, 0.0f };
};
} // namespace aur
