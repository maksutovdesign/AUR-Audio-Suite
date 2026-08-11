#pragma once

#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace aur
{
/** First-order allpass with a tunable break frequency (for phasers). */
struct Allpass1
{
    float a = 0.0f, z = 0.0f;
    double fs = 48000.0;

    void prepare (double sampleRate) { fs = sampleRate; z = 0.0f; }
    void reset() { z = 0.0f; }

    void setFrequency (float hz)
    {
        const double t = std::tan (M_PI * (double) hz / fs);
        a = (float) ((t - 1.0) / (t + 1.0));
    }

    float process (float x)
    {
        const float y = a * x + z;
        z = x - a * y;
        return y;
    }
};
} // namespace aur
