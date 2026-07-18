#pragma once

#include <cmath>
#include <algorithm>

namespace aur { namespace syn
{
/** Anti-aliased virtual-analog oscillator (PolyBLEP).

    Shapes: 0=Saw, 1=Pulse (pulse-width), 2=Triangle, 3=Sine.
    Phase is normalised to [0,1). Saw/Pulse use a 2-sample PolyBLEP residual to
    cancel the discontinuity's aliasing; Triangle integrates the corrected pulse
    (leaky integrator); Sine is exact. This is the same "alias-free core" idea
    as the effects line's ADAA — clean highs at any pitch. */
struct PolyBlepOsc
{
    void prepare (double sampleRate) { sr = sampleRate; }
    void reset (double startPhase = 0.0) { phase = startPhase; triState = 0.0; }

    void setFrequency (double hz) { inc = hz / sr; }
    void setShape (int s)         { shape = s; }
    void setPulseWidth (float pw) { width = std::clamp (pw, 0.05f, 0.95f); }

    inline float next()
    {
        const double t  = phase;
        const double dt = inc;
        float out = 0.0f;

        switch (shape)
        {
            case 0: // Saw
            {
                double v = 2.0 * t - 1.0;
                v -= polyBlep (t, dt);
                out = (float) v;
            } break;

            case 1: // Pulse / square with pulse width
            {
                double v = t < (double) width ? 1.0 : -1.0;
                v += polyBlep (t, dt);
                double t2 = t + (1.0 - (double) width);
                if (t2 >= 1.0) t2 -= 1.0;
                v -= polyBlep (t2, dt);
                out = (float) v;
            } break;

            case 2: // Triangle (leaky-integrated square)
            {
                double v = t < 0.5 ? 1.0 : -1.0;
                v += polyBlep (t, dt);
                double t2 = t + 0.5;
                if (t2 >= 1.0) t2 -= 1.0;
                v -= polyBlep (t2, dt);
                triState += 4.0 * dt * v;   // integrate; 4*dt keeps amplitude ~unity
                triState *= 0.9992;         // leak to remove DC
                out = (float) triState;
            } break;

            default: // Sine
                out = (float) std::sin (6.283185307179586 * t);
                break;
        }

        phase += inc;
        if (phase >= 1.0) phase -= 1.0;
        return out;
    }

private:
    // 2-sample PolyBLEP residual around a phase discontinuity.
    static inline double polyBlep (double t, double dt)
    {
        if (t < dt)             { t /= dt;            return t + t - t * t - 1.0; }
        if (t > 1.0 - dt)       { t = (t - 1.0) / dt; return t * t + t + t + 1.0; }
        return 0.0;
    }

    double sr    = 44100.0;
    double phase = 0.0;
    double inc   = 0.0;
    double triState = 0.0;
    float  width = 0.5f;
    int    shape = 0;
};
}} // namespace aur::syn
