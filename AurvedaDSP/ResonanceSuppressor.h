#pragma once

#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include <array>
#include <atomic>
#include "Biquad.h"
#include "SimpleFFT.h"

namespace aur
{
/**
    ResonanceSuppressor — the core of CLARITY.

    Dynamic, perceptual resonance suppression. An FFT analyser measures the
    running spectrum and folds it onto a Bark-like (critical-band) grid; any
    band that sticks out ABOVE the local spectral trend (the average across
    bands) is treated as a resonance and ducked by a matching peaking cut, only
    while and where it protrudes. Nothing is boosted, and flat/broadband
    content is left alone — so it removes harshness and mud without the
    "processed" sound of static EQ.

    Detection is FFT-based (fine frequency resolution, like the best tools);
    the audio path is a cascade of time-domain peaking cuts (clean
    reconstruction, no overlap-add artefacts). Realtime-safe: fixed arrays,
    in-place biquad retuning, no allocation. Pure C++ (no JUCE) so it can be
    unit-tested offline.
*/
class ResonanceSuppressor
{
public:
    static constexpr size_t kBands = 22;
    static constexpr size_t kMaxCh = 2;
    static constexpr int    kFFT   = 1024;
    static constexpr int    kHop   = 256;
    static constexpr int    kHalf  = kFFT / 2;

    void prepare (double sampleRate, int /*maxBlock*/, int numChannels)
    {
        fs = sampleRate;
        channels = (numChannels < (int) kMaxCh) ? (size_t) numChannels : kMaxCh;

        const double lo = 160.0, hi = 14000.0;
        for (size_t b = 0; b < kBands; ++b)
        {
            const double t = (double) b / (double) (kBands - 1);
            centre[b] = lo * std::pow (hi / lo, t);
            for (size_t c = 0; c < kMaxCh; ++c)
                cut[b][c].setPeaking (fs, centre[b], sharpQ, 0.0);
        }

        // Hann analysis window.
        for (int i = 0; i < kFFT; ++i)
            win[(size_t) i] = 0.5f - 0.5f * (float) std::cos (2.0 * M_PI * i / (kFFT - 1));

        // Map each FFT bin to its nearest band (log distance); track band bin
        // counts and each band's nearest bin (fallback for empty bands).
        for (size_t b = 0; b < kBands; ++b) { bandCnt[b] = 0; nearestBin[b] = 1; }
        std::array<double, kBands> bestDist;
        for (size_t b = 0; b < kBands; ++b) bestDist[b] = 1e30;
        for (int k = 1; k <= kHalf; ++k)
        {
            const double f = (double) k * fs / (double) kFFT;
            size_t bb = 0; double best = 1e30;
            for (size_t b = 0; b < kBands; ++b)
            {
                const double d = std::abs (std::log (f) - std::log (centre[b]));
                if (d < best) { best = d; bb = b; }
            }
            binBand[(size_t) k] = (int) bb;
            bandCnt[bb]++;
            if (best < bestDist[bb]) { bestDist[bb] = best; nearestBin[bb] = k; }
        }

        // Per-hop envelope coefficients.
        envAtk = (float) std::exp (-(double) kHop / (0.010 * fs));
        envRel = (float) std::exp (-(double) kHop / (0.120 * fs));
        redAtk = (float) std::exp (-(double) kHop / (0.005 * fs));
        redRel = (float) std::exp (-(double) kHop / (0.060 * fs));
        reset();
    }

    void reset()
    {
        for (size_t b = 0; b < kBands; ++b)
        {
            for (size_t c = 0; c < kMaxCh; ++c) cut[b][c].reset();
            env[b] = 1.0e-6f;
            curRed[b] = 0.0f;
        }
        for (int i = 0; i < kFFT; ++i) ring[(size_t) i] = 0.0f;
        ringPos = 0;
        hopCount = 0;
    }

    void setParameters (float depth, float sensitivityDb, float sharpness, float mixPercent)
    {
        depthScale = depth / 100.0f * 1.6f;
        threshold  = sensitivityDb;
        mix        = mixPercent / 100.0f;

        const float newQ = 1.5f + (sharpness / 100.0f) * 8.5f;
        if (std::abs (newQ - sharpQ) > 0.01f)
        {
            sharpQ = newQ;
            for (size_t b = 0; b < kBands; ++b)
                for (size_t c = 0; c < kMaxCh; ++c)
                    cut[b][c].setPeaking (fs, centre[b], sharpQ, -(double) curRed[b]);
        }
    }

    // ---- Visualisation snapshot (lock-free; audio writes, UI reads) ----
    size_t numBands()            const { return kBands; }
    float  vizCentre (size_t i)  const { return (float) centre[i]; }
    float  vizLevelDb (size_t i) const { return vizLevel[i].load (std::memory_order_relaxed); }
    float  vizReductionDb (size_t i) const { return vizRed[i].load (std::memory_order_relaxed); }

    void process (float* const* data, int numChIn, int numSamples, bool delta = false)
    {
        const size_t nc = ((size_t) numChIn < channels) ? (size_t) numChIn : channels;

        for (int n = 0; n < numSamples; ++n)
        {
            float mono = 0.0f;
            for (size_t c = 0; c < nc; ++c) mono += data[c][n];
            mono /= (float) (nc > 0 ? nc : 1);

            ring[(size_t) ringPos] = mono;
            ringPos = (ringPos + 1) & (kFFT - 1);

            if (++hopCount >= kHop)
            {
                hopCount = 0;
                analyseAndUpdate();
            }

            for (size_t c = 0; c < nc; ++c)
            {
                const float dry = data[c][n];
                float wet = dry;
                for (size_t b = 0; b < kBands; ++b)
                    wet = cut[b][c].process (wet);
                data[c][n] = delta ? (dry - wet) : (dry * (1.0f - mix) + wet * mix);
            }
        }
    }

private:
    void analyseAndUpdate()
    {
        // Assemble windowed frame (oldest sample first) and transform.
        for (int i = 0; i < kFFT; ++i)
        {
            const size_t si = (size_t) i;
            re[si] = (double) (ring[(size_t) ((ringPos + i) & (kFFT - 1))] * win[si]);
            im[si] = 0.0;
        }
        SimpleFFT::forward (re.data(), im.data(), kFFT);

        for (int k = 1; k <= kHalf; ++k)
        {
            const size_t sk = (size_t) k;
            mag[sk] = std::sqrt (re[sk] * re[sk] + im[sk] * im[sk]);
        }

        // Fold bins onto bands (mean magnitude per band).
        std::array<double, kBands> acc {};
        for (int k = 1; k <= kHalf; ++k)
            acc[(size_t) binBand[(size_t) k]] += mag[(size_t) k];

        float dB[kBands];
        float mean = 0.0f;
        for (size_t b = 0; b < kBands; ++b)
        {
            const double raw = (bandCnt[b] > 0) ? acc[b] / (double) bandCnt[b]
                                                : mag[(size_t) nearestBin[b]];
            const float target = (float) raw;
            const float coeff = target > env[b] ? envAtk : envRel;
            env[b] = coeff * (env[b] - target) + target;

            dB[b] = 20.0f * std::log10 (env[b] + 1.0e-9f);
            mean += dB[b];
        }
        mean /= (float) kBands;

        for (size_t b = 0; b < kBands; ++b)
        {
            const float excess = dB[b] - mean - threshold;
            float target = excess > 0.0f ? excess * depthScale : 0.0f;
            if (target > maxReduction) target = maxReduction;

            const float coeff = target > curRed[b] ? redAtk : redRel;
            curRed[b] = coeff * (curRed[b] - target) + target;

            for (size_t c = 0; c < channels; ++c)
                cut[b][c].setPeaking (fs, centre[b], sharpQ, -(double) curRed[b]);

            vizLevel[b].store (dB[b], std::memory_order_relaxed);
            vizRed[b].store   (curRed[b], std::memory_order_relaxed);
        }
    }

    double fs = 44100.0;
    size_t channels = 2;

    std::array<double, kBands> centre {};
    std::array<std::array<Biquad, kMaxCh>, kBands> cut {};
    std::array<float, kBands> env {};
    std::array<float, kBands> curRed {};
    std::array<std::atomic<float>, kBands> vizLevel {};
    std::array<std::atomic<float>, kBands> vizRed {};

    // FFT analysis state.
    std::array<float, kFFT>  win {};
    std::array<float, kFFT>  ring {};
    std::array<double, kFFT> re {};
    std::array<double, kFFT> im {};
    std::array<double, kHalf + 1> mag {};
    std::array<int, kHalf + 1> binBand {};
    std::array<int, kBands> nearestBin {};
    std::array<int, kBands> bandCnt {};
    int ringPos = 0, hopCount = 0;

    float envAtk = 0.0f, envRel = 0.0f, redAtk = 0.0f, redRel = 0.0f;
    float depthScale = 0.8f, threshold = 3.0f, mix = 1.0f, sharpQ = 4.0f;
    static constexpr float maxReduction = 18.0f;
};
} // namespace aur
