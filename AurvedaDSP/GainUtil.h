#pragma once

#include <cmath>

namespace aur
{
/** Utility: gain, balance, stereo width (M/S), mono sum, phase invert. */
class GainUtil
{
public:
    void prepare (double, int) {}
    void reset() {}

    void setParameters (float gainDb, float balance, float widthPct, bool monoIn, bool phaseIn)
    {
        const float g = std::pow (10.0f, gainDb / 20.0f);
        const float b = balance < -1.0f ? -1.0f : (balance > 1.0f ? 1.0f : balance);
        gainL = g * (b <= 0.0f ? 1.0f : 1.0f - b);
        gainR = g * (b >= 0.0f ? 1.0f : 1.0f + b);
        width = widthPct / 100.0f;
        mono = monoIn; phase = phaseIn;
    }

    void process (float* const* data, int numCh, int numSamples)
    {
        const float ps = phase ? -1.0f : 1.0f;
        if (numCh < 2)
        {
            for (int i = 0; i < numSamples; ++i) data[0][i] *= gainL * ps;
            return;
        }
        for (int i = 0; i < numSamples; ++i)
        {
            float l = data[0][i] * gainL * ps;
            float r = data[1][i] * gainR * ps;
            const float mid  = 0.5f * (l + r);
            const float side = 0.5f * (l - r) * width;
            l = mid + side; r = mid - side;
            if (mono) { l = mid; r = mid; }
            data[0][i] = l; data[1][i] = r;
        }
    }

private:
    float gainL = 1.0f, gainR = 1.0f, width = 1.0f;
    bool mono = false, phase = false;
};
} // namespace aur
