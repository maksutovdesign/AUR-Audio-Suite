#pragma once

#include <cmath>

namespace aur
{
/** Simple sine LFO with a phase accumulator. */
struct LFO
{
    double phase = 0.0, inc = 0.0, fs = 48000.0;

    void prepare (double sampleRate) { fs = sampleRate; phase = 0.0; }
    void setRate (float hz) { inc = (double) hz / fs; }

    /** Advance and return sine in [-1,1]. */
    float next()
    {
        const float v = (float) std::sin (2.0 * M_PI * phase);
        phase += inc; if (phase >= 1.0) phase -= 1.0;
        return v;
    }

    /** Sine at a fixed phase offset (0..1) without advancing — for stereo. */
    float at (double offset) const
    {
        double p = phase + offset; if (p >= 1.0) p -= 1.0;
        return (float) std::sin (2.0 * M_PI * p);
    }
};
} // namespace aur
