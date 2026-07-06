#pragma once

#include <cmath>
#include <array>
#include "Biquad.h"

namespace aur
{
/**
    ResonanceSuppressor — the core of CLARITY.

    Dynamic, perceptual resonance suppression. A bank of Bark-spaced bandpass
    detectors measures the running spectrum; any band that sticks out ABOVE the
    local spectral trend (its neighbours) is treated as a resonance and ducked
    by a matching peaking cut, only while and where it protrudes. Nothing is
    boosted, and flat/broadband content is left alone — so it removes harshness
    and mud without the "processed" sound of static EQ.

    Why "perceptual": bands sit on a Bark-like (critical-band) grid and the
    baseline is the smoothed average across neighbours (a masking-style
    reference), rather than an absolute threshold.

    Realtime-safe: fixed arrays, in-place biquad retuning, no allocation.
    Pure C++ (no JUCE) so it can be unit-tested offline.
*/
class ResonanceSuppressor
{
public:
    static constexpr size_t kBands = 22;
    static constexpr size_t kMaxCh = 2;

    void prepare (double sampleRate, int /*maxBlock*/, int numChannels)
    {
        fs = sampleRate;
        channels = (numChannels < (int) kMaxCh) ? (size_t) numChannels : kMaxCh;

        // Bark-ish log-spaced centres, 160 Hz .. 14 kHz.
        const double lo = 160.0, hi = 14000.0;
        for (size_t b = 0; b < kBands; ++b)
        {
            const double t = (double) b / (double) (kBands - 1);
            centre[b] = lo * std::pow (hi / lo, t);
            detect[b].setBandpass (fs, centre[b], detectorQ);
            for (size_t c = 0; c < kMaxCh; ++c)
                cut[b][c].setPeaking (fs, centre[b], sharpQ, 0.0);
        }

        atkC    = (float) std::exp (-1.0 / (0.002 * fs));   // 2 ms
        relC    = (float) std::exp (-1.0 / (0.080 * fs));   // 80 ms
        redAtkC = (float) std::exp (-1.0 / (0.005 * fs));
        redRelC = (float) std::exp (-1.0 / (0.060 * fs));
        reset();
    }

    void reset()
    {
        for (size_t b = 0; b < kBands; ++b)
        {
            detect[b].reset();
            for (size_t c = 0; c < kMaxCh; ++c) cut[b][c].reset();
            env[b]    = 1.0e-6f;
            curRed[b] = 0.0f;
        }
        sampleCounter = 0;
    }

    /** depth 0..100, sensitivityDb (threshold above baseline), sharpness 0..100 (Q). */
    void setParameters (float depth, float sensitivityDb, float sharpness, float mixPercent)
    {
        depthScale = depth / 100.0f * 1.6f;          // excess dB → reduction dB
        threshold  = sensitivityDb;
        mix        = mixPercent / 100.0f;

        const float newQ = 1.5f + (sharpness / 100.0f) * 8.5f; // 1.5 .. 10
        if (std::abs (newQ - sharpQ) > 0.01f)
        {
            sharpQ = newQ;
            for (size_t b = 0; b < kBands; ++b)
                for (size_t c = 0; c < kMaxCh; ++c)
                    cut[b][c].setPeaking (fs, centre[b], sharpQ, -(double) curRed[b]);
        }
    }

    /** In-place processing. data[ch][n]. If delta==true, outputs only what was removed. */
    void process (float* const* data, int numChIn, int numSamples, bool delta = false)
    {
        const size_t nc = ((size_t) numChIn < channels) ? (size_t) numChIn : channels;

        for (int n = 0; n < numSamples; ++n)
        {
            // Mono detection signal.
            float mono = 0.0f;
            for (size_t c = 0; c < nc; ++c) mono += data[c][n];
            mono /= (float) (nc > 0 ? nc : 1);

            // Per-band detector envelopes (peak follower).
            for (size_t b = 0; b < kBands; ++b)
            {
                const float bp = std::abs (detect[b].process (mono));
                const float coeff = bp > env[b] ? atkC : relC;
                env[b] = coeff * (env[b] - bp) + bp;
            }

            // Control-rate: recompute reduction targets + retune cut filters.
            if (--sampleCounter <= 0)
            {
                sampleCounter = controlHop;
                updateReductions();
            }

            // Audio path: cascade of dynamic peaking cuts.
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
    void updateReductions()
    {
        float dB[kBands];
        float mean = 0.0f;
        for (size_t b = 0; b < kBands; ++b)
        {
            dB[b] = 20.0f * std::log10 (env[b] + 1.0e-9f);
            mean += dB[b];
        }
        mean /= (float) kBands;

        for (size_t b = 0; b < kBands; ++b)
        {
            const float excess = dB[b] - mean - threshold;      // how far it protrudes
            float target = excess > 0.0f ? excess * depthScale : 0.0f;
            if (target > maxReduction) target = maxReduction;

            const float coeff = target > curRed[b] ? redAtkC : redRelC;
            curRed[b] = coeff * (curRed[b] - target) + target;

            for (size_t c = 0; c < channels; ++c)
                cut[b][c].setPeaking (fs, centre[b], sharpQ, -(double) curRed[b]);
        }
    }

    double fs = 44100.0;
    size_t channels = 2;

    std::array<double, kBands> centre {};
    std::array<Biquad, kBands> detect {};
    std::array<std::array<Biquad, kMaxCh>, kBands> cut {};
    std::array<float, kBands> env {};
    std::array<float, kBands> curRed {};

    float atkC = 0.0f, relC = 0.0f, redAtkC = 0.0f, redRelC = 0.0f;
    float depthScale = 0.8f, threshold = 3.0f, mix = 1.0f;
    float detectorQ = 4.0f, sharpQ = 4.0f;
    static constexpr float maxReduction = 18.0f;

    int controlHop = 32;
    int sampleCounter = 0;
};
} // namespace aur
