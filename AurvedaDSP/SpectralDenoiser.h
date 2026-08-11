#pragma once

#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include <array>
#include <atomic>
#include "SimpleFFT.h"

namespace aur
{
/**
    Spectral de-noiser (STFT). Learns a per-bin noise magnitude profile from a
    quiet passage, then attenuates bins that sit near/below that profile via a
    soft spectral gate — reducing broadband hiss/noise while preserving signal.

    Weighted overlap-add: 1024-pt FFT, 256 hop (75%), Hann analysis + synthesis
    windows (COLA). Latency = fftSize. Realtime-safe (fixed buffers), pure C++.
*/
class SpectralDenoiser
{
public:
    static constexpr int   kFFT  = 1024;
    static constexpr int   kHop  = 256;
    static constexpr int   kBins = kFFT / 2 + 1;
    static constexpr size_t kMaxCh = 2;

    void prepare (double sampleRate, int numChannels)
    {
        fs = sampleRate;
        channels = (numChannels < (int) kMaxCh) ? (size_t) numChannels : kMaxCh;
        for (int i = 0; i < kFFT; ++i)
            win[(size_t) i] = 0.5f - 0.5f * (float) std::cos (2.0 * M_PI * i / (kFFT - 1));
        // COLA normalisation for Hann analysis+synthesis at 75% overlap.
        norm = (float) (kHop) / (0.375f * kFFT); // ≈ 1/1.5
        for (int b = 0; b < kBins; ++b) profile[(size_t) b] = 1e-4f;
        learnFrames = 0;
        reset();
    }

    void reset()
    {
        for (size_t c = 0; c < kMaxCh; ++c)
        {
            inRing[c].fill (0.0f);
            outRing[c].fill (0.0f);
            pos[c] = 0; count[c] = 0;
        }
    }

    void setParameters (float amountPct, float sensitivity)
    {
        floor = std::pow (10.0f, -(amountPct / 100.0f) * 40.0f / 20.0f); // amount → residual floor
        sens = 1.0f + sensitivity / 100.0f * 5.0f;                        // 1 .. 6
    }

    /** Request a learn pass; the reset happens on the audio thread (safe). */
    void startLearn (float seconds = 1.0f)
    {
        learnTarget = (int) (seconds * fs / kHop);
        learnRequest.store (true);
    }
    bool isLearning() const { return learning.load(); }

    void process (float* const* data, int numChIn, int numSamples)
    {
        const size_t nc = ((size_t) numChIn < channels) ? (size_t) numChIn : channels;
        for (int n = 0; n < numSamples; ++n)
            for (size_t c = 0; c < nc; ++c)
            {
                const float in = data[c][n];
                const int p = pos[c];
                const float out = outRing[c][(size_t) p];
                outRing[c][(size_t) p] = 0.0f;      // consumed
                inRing[c][(size_t) p] = in;
                pos[c] = (p + 1) % kFFT;
                data[c][n] = out;

                if (++count[c] >= kHop)
                {
                    count[c] = 0;
                    processFrame (c);
                }
            }
    }

private:
    void processFrame (size_t c)
    {
        const int p = pos[c];
        for (int i = 0; i < kFFT; ++i)
        {
            re[(size_t) i] = (double) (inRing[c][(size_t) ((p + i) % kFFT)] * win[(size_t) i]);
            im[(size_t) i] = 0.0;
        }
        SimpleFFT::forward (re.data(), im.data(), kFFT);

        // Begin a requested learn pass on the audio thread (channel 0 only).
        if (c == 0 && learnRequest.exchange (false))
        {
            for (int b = 0; b < kBins; ++b) learnAcc[(size_t) b] = 0.0;
            learnFrames = 0;
            learning.store (true);
        }

        // Only channel 0 updates the learned profile (mono-ish noise estimate).
        const bool learn = learning.load();
        for (int b = 0; b < kBins; ++b)
        {
            const double mag = std::sqrt (re[(size_t) b] * re[(size_t) b] + im[(size_t) b] * im[(size_t) b]);

            if (learn && c == 0) learnAcc[(size_t) b] += mag;

            const double thr = (double) profile[(size_t) b] * (double) sens;
            double g;
            if (mag >= thr) g = 1.0;
            else { const double r = mag / (thr + 1e-12); g = (double) floor + (1.0 - (double) floor) * r * r; }

            re[(size_t) b] *= g; im[(size_t) b] *= g;
            if (b > 0 && b < kFFT / 2)   // mirror to negative frequencies
            {
                re[(size_t) (kFFT - b)] *= g;
                im[(size_t) (kFFT - b)] *= g;
            }
        }

        if (learn && c == 0 && ++learnFrames >= learnTarget)
        {
            for (int b = 0; b < kBins; ++b)
                profile[(size_t) b] = (float) (learnAcc[(size_t) b] / (double) (learnFrames > 0 ? learnFrames : 1));
            learning.store (false);
        }

        SimpleFFT::inverse (re.data(), im.data(), kFFT);

        for (int i = 0; i < kFFT; ++i)
            outRing[c][(size_t) ((p + i) % kFFT)] += (float) re[(size_t) i] * win[(size_t) i] * norm;
    }

    double fs = 48000.0;
    size_t channels = 2;
    std::array<float, kFFT> win {};
    float norm = 0.6667f, floor = 0.1f, sens = 2.0f;

    std::array<std::array<float, kFFT>, kMaxCh> inRing {}, outRing {};
    std::array<int, kMaxCh> pos {}, count {};

    std::array<double, kFFT> re {}, im {};
    std::array<float, kBins> profile {};
    std::array<double, kBins> learnAcc {};
    std::atomic<bool> learning { false };
    std::atomic<bool> learnRequest { false };
    int learnFrames = 0, learnTarget = 180;
};
} // namespace aur
