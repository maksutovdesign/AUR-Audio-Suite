#pragma once

#include <array>
#include "Biquad.h"

namespace aur
{
/**
    De-hummer. Removes mains hum by placing a bank of narrow peaking cuts at the
    fundamental (50/60 Hz) and its harmonics. Depth and Q are shared across the
    harmonic bank. Realtime-safe (in-place coeff updates), pure C++.
*/
class DeHummer
{
public:
    static constexpr int kMaxH = 8;
    static constexpr size_t kMaxCh = 2;

    void prepare (double sampleRate, int numChannels)
    {
        fs = sampleRate;
        channels = (numChannels < (int) kMaxCh) ? (size_t) numChannels : kMaxCh;
        reset();
        setParameters (50.0f, 4, 24.0f, 20.0f);
    }

    void reset()
    {
        for (auto& harm : notch)
            for (auto& f : harm) f.reset();
    }

    void setParameters (float baseFreq, int harmonics, float depthDb, float Q)
    {
        active = harmonics < 1 ? 1 : (harmonics > kMaxH ? kMaxH : harmonics);
        for (int k = 0; k < kMaxH; ++k)
        {
            const double f = (double) baseFreq * (k + 1);
            if (k < active && f < fs * 0.45)
                for (size_t c = 0; c < kMaxCh; ++c)
                    notch[(size_t) k][c].setPeaking (fs, f, Q, -depthDb);
            else
                for (size_t c = 0; c < kMaxCh; ++c)
                    notch[(size_t) k][c].setPeaking (fs, f < fs * 0.45 ? f : 1000.0, Q, 0.0);
        }
    }

    void process (float* const* data, int numChIn, int numSamples)
    {
        const size_t nc = ((size_t) numChIn < channels) ? (size_t) numChIn : channels;
        for (int n = 0; n < numSamples; ++n)
            for (size_t c = 0; c < nc; ++c)
            {
                float x = data[c][n];
                for (int k = 0; k < active; ++k)
                    x = notch[(size_t) k][c].process (x);
                data[c][n] = x;
            }
    }

private:
    double fs = 48000.0;
    size_t channels = 2;
    std::array<std::array<Biquad, kMaxCh>, kMaxH> notch {};
    int active = 4;
};
} // namespace aur
