#pragma once

#include <cmath>

namespace aur
{
/**
    Minimal realtime-safe biquad (Transposed Direct Form II). Coefficients are
    computed in place — no allocation — so it is safe to retune inside the audio
    callback. One instance holds one channel's state.
*/
struct Biquad
{
    double b0 = 1.0, b1 = 0.0, b2 = 0.0, a1 = 0.0, a2 = 0.0;
    double z1 = 0.0, z2 = 0.0;

    void reset() { z1 = z2 = 0.0; }

    void setPeaking (double fs, double freq, double Q, double gainDb)
    {
        const double A  = std::pow (10.0, gainDb / 40.0);
        const double w0 = 2.0 * M_PI * freq / fs;
        const double cw = std::cos (w0), sw = std::sin (w0);
        const double alpha = sw / (2.0 * Q);
        const double a0 = 1.0 + alpha / A;
        b0 = (1.0 + alpha * A) / a0;
        b1 = (-2.0 * cw)       / a0;
        b2 = (1.0 - alpha * A) / a0;
        a1 = (-2.0 * cw)       / a0;
        a2 = (1.0 - alpha / A) / a0;
    }

    void setBandpass (double fs, double freq, double Q)
    {
        const double w0 = 2.0 * M_PI * freq / fs;
        const double cw = std::cos (w0), sw = std::sin (w0);
        const double alpha = sw / (2.0 * Q);
        const double a0 = 1.0 + alpha;
        b0 =  alpha / a0;
        b1 =  0.0;
        b2 = -alpha / a0;
        a1 = (-2.0 * cw) / a0;
        a2 = (1.0 - alpha) / a0;
    }

    inline float process (float x) noexcept
    {
        const double y = b0 * (double) x + z1;
        z1 = b1 * (double) x - a1 * y + z2;
        z2 = b2 * (double) x - a2 * y;
        return (float) y;
    }
};
} // namespace aur
