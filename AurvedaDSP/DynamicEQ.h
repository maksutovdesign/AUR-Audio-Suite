#pragma once

#include <cmath>
#include <array>
#include <atomic>
#include "Biquad.h"

namespace aur
{
/**
    Dynamic EQ — three bands, each a peaking filter whose cut is driven by that
    band's own level. When a band exceeds its threshold it is attenuated by up
    to `range` dB (dynamic de-emphasis, like a per-band compressor on the EQ);
    below threshold it stays flat. Detection via bandpass envelope; coefficients
    updated at control rate to stay click-free. Realtime-safe, pure C++.
*/
class DynamicEQ
{
public:
    static constexpr size_t NB = 3;
    static constexpr size_t kMaxCh = 2;

    void prepare (double sampleRate, int numChannels)
    {
        fs = sampleRate;
        channels = (numChannels < (int) kMaxCh) ? (size_t) numChannels : kMaxCh;
        atk = (float) std::exp (-1.0 / (0.005 * fs));
        rel = (float) std::exp (-1.0 / (0.080 * fs));
        for (size_t b = 0; b < NB; ++b) { env[b] = 1e-6f; red[b] = 0.0f; }
        reset();
    }

    void reset()
    {
        for (size_t b = 0; b < NB; ++b)
        {
            det[b].reset();
            for (size_t c = 0; c < kMaxCh; ++c) cut[b][c].reset();
        }
        counter = 0;
    }

    void setBand (size_t b, float freq, float thresholdDb, float rangeDb, float Q)
    {
        if (b >= NB) return;
        centre[b] = freq; thr[b] = thresholdDb; rng[b] = rangeDb; q[b] = Q;
        det[b].setBandpass (fs, freq, Q);
    }

    void process (float* const* data, int numChIn, int numSamples)
    {
        const size_t nc = ((size_t) numChIn < channels) ? (size_t) numChIn : channels;

        for (int n = 0; n < numSamples; ++n)
        {
            float mono = 0.0f;
            for (size_t c = 0; c < nc; ++c) mono += data[c][n];
            mono /= (float) (nc > 0 ? nc : 1);

            for (size_t b = 0; b < NB; ++b)
            {
                const float bp = std::abs (det[b].process (mono));
                const float coeff = bp > env[b] ? atk : rel;
                env[b] = coeff * (env[b] - bp) + bp;
            }

            if (--counter <= 0)
            {
                counter = 32;
                for (size_t b = 0; b < NB; ++b)
                {
                    const float envDb = 20.0f * std::log10 (env[b] + 1e-9f);
                    float target = envDb - thr[b];
                    target = target > 0.0f ? (target < rng[b] ? target : rng[b]) : 0.0f;
                    const float c = target > red[b] ? 0.5f : 0.1f;
                    red[b] += c * (target - red[b]);
                    for (size_t ch = 0; ch < channels; ++ch)
                        cut[b][ch].setPeaking (fs, centre[b], q[b], -(double) red[b]);
                    viz[b].store (red[b], std::memory_order_relaxed);
                }
            }

            for (size_t c = 0; c < nc; ++c)
            {
                float x = data[c][n];
                for (size_t b = 0; b < NB; ++b) x = cut[b][c].process (x);
                data[c][n] = x;
            }
        }
    }

    float getReductionDb (size_t b) const { return b < NB ? viz[b].load (std::memory_order_relaxed) : 0.0f; }
    float bandFreq (size_t b) const { return b < NB ? centre[b] : 0.0f; }

private:
    double fs = 48000.0;
    size_t channels = 2;
    std::array<Biquad, NB> det {};
    std::array<std::array<Biquad, kMaxCh>, NB> cut {};
    std::array<float, NB> env {}, red {}, centre { 200.f, 1000.f, 6000.f }, thr {}, rng {}, q { 2.f, 2.f, 2.f };
    std::array<std::atomic<float>, NB> viz {};
    float atk = 0.f, rel = 0.f;
    int counter = 0;
};
} // namespace aur
