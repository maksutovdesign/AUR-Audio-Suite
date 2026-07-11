#pragma once

#include "LFO.h"

namespace aur
{
/** Tremolo / auto-pan. Stereo phase 0 = tremolo (both channels together),
    1 = auto-pan (channels 180° apart). */
class Tremolo
{
public:
    void prepare (double sampleRate) { lfo.prepare (sampleRate); }
    void reset() {}

    void setParameters (float rateHz, float depthPct, float stereoPct)
    {
        lfo.setRate (rateHz);
        depth = depthPct / 100.0f;
        stereoOffset = (stereoPct / 100.0f) * 0.5f;   // up to half a cycle (180°)
    }

    void process (float* L, float* R, int numSamples)
    {
        for (int n = 0; n < numSamples; ++n)
        {
            const float mL = lfo.at (0.0) * 0.5f + 0.5f;
            const float mR = lfo.at (stereoOffset) * 0.5f + 0.5f;
            lfo.next();
            L[n] *= (1.0f - depth * mL);
            R[n] *= (1.0f - depth * mR);
        }
    }

private:
    LFO lfo;
    float depth = 0.5f, stereoOffset = 0.0f;
};
} // namespace aur
