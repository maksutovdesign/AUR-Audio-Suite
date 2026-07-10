#pragma once

#include <cmath>
#include <array>
#include "Biquad.h"

namespace aur
{
/**
    Stereo imager. Mid/Side width control with a "mono-maker" (frequencies
    below a cutoff are collapsed to mono by high-passing the side signal —
    keeps low end centred and mono-compatible), plus L/R balance. Realtime-safe.
*/
class StereoImager
{
public:
    void prepare (double sampleRate, int /*numCh*/)
    {
        fs = sampleRate;
        for (auto& f : sideHP) f.reset();
        setParameters (100.0f, 0.0f, 0.0f);
    }

    void reset() { for (auto& f : sideHP) f.reset(); }

    /** widthPct 0..200, monoBelowHz, balance -1..+1 */
    void setParameters (float widthPct, float monoBelowHz, float balance)
    {
        width = widthPct / 100.0f;
        monoOn = monoBelowHz > 21.0f;
        if (monoOn)
            for (auto& f : sideHP)
                f.setHighpass (fs, monoBelowHz, 0.707);
        // Equal-power-ish balance.
        const float b = balance < -1.0f ? -1.0f : (balance > 1.0f ? 1.0f : balance);
        gainL = b <= 0.0f ? 1.0f : (1.0f - b);
        gainR = b >= 0.0f ? 1.0f : (1.0f + b);
    }

    void process (float* left, float* right, int numSamples)
    {
        for (int n = 0; n < numSamples; ++n)
        {
            const float l = left[n], r = right[n];
            float mid  = 0.5f * (l + r);
            float side = 0.5f * (l - r) * width;

            if (monoOn) side = sideHP[0].process (side); // remove low-freq side → mono lows

            left[n]  = (mid + side) * gainL;
            right[n] = (mid - side) * gainR;
        }
    }

private:
    double fs = 48000.0;
    std::array<Biquad, 1> sideHP {};
    float width = 1.0f, gainL = 1.0f, gainR = 1.0f;
    bool monoOn = false;
};
} // namespace aur
