#pragma once

#include <cmath>
#include <array>
#include <atomic>
#include "Biquad.h"

namespace aur
{
/**
    Split-band de-esser. A high-pass detector tracks sibilant energy; when it
    exceeds the threshold, a high-shelf cut is applied at the same frequency by
    up to `range` dB — only while the "ess" is present. A Listen mode outputs
    just the targeted band so the user can dial in the frequency. Realtime-safe.
*/
class DeEsser
{
public:
    static constexpr size_t kMaxCh = 2;

    void prepare (double sampleRate, int numChannels)
    {
        fs = sampleRate;
        channels = (numChannels < (int) kMaxCh) ? (size_t) numChannels : kMaxCh;
        atk = (float) std::exp (-1.0 / (0.002 * fs));   // 2 ms
        rel = (float) std::exp (-1.0 / (0.040 * fs));   // 40 ms
        env = 1e-6f; red = 0.0f;
        setParameters (6500.0f, -30.0f, 10.0f);
        reset();
    }

    void reset()
    {
        det.reset();
        for (size_t c = 0; c < kMaxCh; ++c) { cut[c].reset(); listen[c].reset(); }
        env = 1e-6f; red = 0.0f; counter = 0;
    }

    void setParameters (float freqHz, float thresholdDb, float rangeDb)
    {
        freq = freqHz; thr = thresholdDb; rng = rangeDb;
        det.setHighpass (fs, freqHz * 0.85, 0.707);
        for (size_t c = 0; c < kMaxCh; ++c)
            listen[c].setHighpass (fs, freqHz * 0.85, 0.707);
    }

    void process (float* const* data, int numChIn, int numSamples, bool listenMode)
    {
        const size_t nc = ((size_t) numChIn < channels) ? (size_t) numChIn : channels;

        for (int n = 0; n < numSamples; ++n)
        {
            float mono = 0.0f;
            for (size_t c = 0; c < nc; ++c) mono += data[c][n];
            mono /= (float) (nc > 0 ? nc : 1);

            const float d = std::abs (det.process (mono));
            const float coeff = d > env ? atk : rel;
            env = coeff * (env - d) + d;

            if (--counter <= 0)
            {
                counter = 32;
                const float envDb = 20.0f * std::log10 (env + 1e-9f);
                float target = envDb - thr;
                target = target > 0.0f ? (target < rng ? target : rng) : 0.0f;
                const float cc = target > red ? 0.5f : 0.15f;
                red += cc * (target - red);
                for (size_t c = 0; c < channels; ++c)
                    cut[c].setHighShelf (fs, freq, 0.707, -(double) red);
                viz.store (red, std::memory_order_relaxed);
            }

            for (size_t c = 0; c < nc; ++c)
                data[c][n] = listenMode ? listen[c].process (data[c][n])
                                        : cut[c].process (data[c][n]);
        }
    }

    float getReductionDb() const { return viz.load (std::memory_order_relaxed); }

private:
    double fs = 48000.0;
    size_t channels = 2;
    Biquad det;
    std::array<Biquad, kMaxCh> cut {}, listen {};
    std::atomic<float> viz { 0.0f };
    float env = 1e-6f, red = 0.0f, atk = 0.f, rel = 0.f;
    float freq = 6500.f, thr = -30.f, rng = 10.f;
    int counter = 0;
};
} // namespace aur
