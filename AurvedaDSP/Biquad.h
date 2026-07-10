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

    void setHighpass (double fs, double freq, double Q)
    {
        const double w0 = 2.0 * M_PI * freq / fs;
        const double cw = std::cos (w0), sw = std::sin (w0);
        const double alpha = sw / (2.0 * Q);
        const double a0 = 1.0 + alpha;
        b0 =  (1.0 + cw) / 2.0 / a0;
        b1 = -(1.0 + cw)       / a0;
        b2 =  (1.0 + cw) / 2.0 / a0;
        a1 = (-2.0 * cw)       / a0;
        a2 =  (1.0 - alpha)    / a0;
    }

    void setHighShelf (double fs, double freq, double Q, double gainDb)
    {
        const double A  = std::pow (10.0, gainDb / 40.0);
        const double w0 = 2.0 * M_PI * freq / fs;
        const double cw = std::cos (w0), sw = std::sin (w0);
        const double alpha = sw / (2.0 * Q);
        const double tsa = 2.0 * std::sqrt (A) * alpha;
        const double a0 =        (A + 1.0) - (A - 1.0) * cw + tsa;
        b0 =        A * ((A + 1.0) + (A - 1.0) * cw + tsa) / a0;
        b1 = -2.0 * A * ((A - 1.0) + (A + 1.0) * cw)       / a0;
        b2 =        A * ((A + 1.0) + (A - 1.0) * cw - tsa) / a0;
        a1 =  2.0 *     ((A - 1.0) - (A + 1.0) * cw)       / a0;
        a2 =           ((A + 1.0) - (A - 1.0) * cw - tsa) / a0;
    }

    void setLowShelf (double fs, double freq, double Q, double gainDb)
    {
        const double A  = std::pow (10.0, gainDb / 40.0);
        const double w0 = 2.0 * M_PI * freq / fs;
        const double cw = std::cos (w0), sw = std::sin (w0);
        const double alpha = sw / (2.0 * Q);
        const double tsa = 2.0 * std::sqrt (A) * alpha;
        const double a0 =        (A + 1.0) + (A - 1.0) * cw + tsa;
        b0 =        A * ((A + 1.0) - (A - 1.0) * cw + tsa) / a0;
        b1 =  2.0 * A * ((A - 1.0) - (A + 1.0) * cw)       / a0;
        b2 =        A * ((A + 1.0) - (A - 1.0) * cw - tsa) / a0;
        a1 = -2.0 *     ((A - 1.0) + (A + 1.0) * cw)       / a0;
        a2 =           ((A + 1.0) + (A - 1.0) * cw - tsa) / a0;
    }

    void setLowpass (double fs, double freq, double Q)
    {
        const double w0 = 2.0 * M_PI * freq / fs;
        const double cw = std::cos (w0), sw = std::sin (w0);
        const double alpha = sw / (2.0 * Q);
        const double a0 = 1.0 + alpha;
        b0 = (1.0 - cw) / 2.0 / a0;
        b1 = (1.0 - cw)       / a0;
        b2 = (1.0 - cw) / 2.0 / a0;
        a1 = (-2.0 * cw)      / a0;
        a2 = (1.0 - alpha)    / a0;
    }

    /** |H(e^{jw})| for drawing frequency responses. */
    double magnitudeAt (double w) const
    {
        const double cw = std::cos (w), sw = std::sin (w);
        const double c2 = std::cos (2.0 * w), s2 = std::sin (2.0 * w);
        const double nr = b0 + b1 * cw + b2 * c2;
        const double ni = -(b1 * sw + b2 * s2);
        const double dr = 1.0 + a1 * cw + a2 * c2;
        const double di = -(a1 * sw + a2 * s2);
        const double num = std::sqrt (nr * nr + ni * ni);
        const double den = std::sqrt (dr * dr + di * di);
        return den > 1.0e-12 ? num / den : 1.0;
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
