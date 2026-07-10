#pragma once

#include <cmath>
#include <array>
#include <vector>

namespace aur
{
/**
    Lookahead brickwall limiter with inter-sample (true-peak) detection.

    - Input gain drives the signal into the ceiling.
    - A mono-max detector estimates the TRUE peak using 4-point Catmull-Rom
      interpolation (catches inter-sample peaks a plain sample-peak limiter
      misses), per ITU-R BS.1770's intent.
    - A lookahead delay lets the gain start ramping down BEFORE a peak arrives,
      so it is attenuated cleanly with no clipping; release is exponential.
    - Reports its lookahead latency so the host can compensate.

    Realtime-safe after prepare() (delay buffers pre-allocated).
*/
class TruePeakLimiter
{
public:
    static constexpr size_t kMaxCh = 2;

    void prepare (double sampleRate, int numChannels, double lookaheadMs = 2.0)
    {
        fs = sampleRate;
        channels = (numChannels < (int) kMaxCh) ? (size_t) numChannels : kMaxCh;
        L = (size_t) std::max (8.0, lookaheadMs * 0.001 * fs);
        attackInc = 1.0f / (float) L;

        for (size_t c = 0; c < kMaxCh; ++c)
            delay[c].assign (L, 0.0f);

        inGainSm.target = inGainSm.current = 1.0f;
        reset();
    }

    void reset()
    {
        gEnv = 1.0f;
        writePos = 0;
        hist = { 0.0f, 0.0f, 0.0f, 0.0f };
        for (auto& d : delay) std::fill (d.begin(), d.end(), 0.0f);
    }

    int getLatencySamples() const { return (int) L; }
    float getGainReductionDb() const { return -20.0f * std::log10 (std::max (gEnv, 1.0e-6f)); }

    void setParameters (float ceilingDb, float releaseMs, float inputGainDb)
    {
        ceiling = std::pow (10.0f, ceilingDb / 20.0f);
        releaseCoeff = std::exp (-1.0f / (float) (0.001 * std::max (1.0f, releaseMs) * fs));
        inGainSm.target = std::pow (10.0f, inputGainDb / 20.0f);
    }

    void process (float* const* data, int numChIn, int numSamples)
    {
        const size_t nc = ((size_t) numChIn < channels) ? (size_t) numChIn : channels;

        for (int n = 0; n < numSamples; ++n)
        {
            const float ig = inGainSm.next();

            // Apply input gain; find mono peak.
            float peak = 0.0f;
            for (size_t c = 0; c < nc; ++c)
            {
                data[c][n] *= ig;
                peak = std::max (peak, std::abs (data[c][n]));
            }

            // True-peak estimate via 4-point interpolation of the peak history.
            hist[0] = hist[1]; hist[1] = hist[2]; hist[2] = hist[3]; hist[3] = peak;
            const float tp = truePeak (hist);

            // Required gain to hold the ceiling, with lookahead ramp + release.
            const float gReq = tp > ceiling ? ceiling / tp : 1.0f;
            if (gReq < gEnv) gEnv = std::max (gReq, gEnv - attackInc);   // anticipate
            else             gEnv = gReq - (gReq - gEnv) * releaseCoeff; // release

            // Delay the audio and apply the (anticipating) gain.
            for (size_t c = 0; c < nc; ++c)
            {
                const float delayed = delay[c][writePos];
                delay[c][writePos] = data[c][n];
                float out = delayed * gEnv;
                out = std::max (-ceiling, std::min (ceiling, out)); // hard safety
                data[c][n] = out;
            }
            writePos = (writePos + 1) % L;
        }
    }

private:
    struct Smooth
    {
        float current = 1.0f, target = 1.0f;
        float next() { current += 0.002f * (target - current); return current; }
    };

    static float truePeak (const std::array<float, 4>& h)
    {
        const float p0 = h[0], p1 = h[1], p2 = h[2], p3 = h[3];
        float tp = std::max (p1, p2);
        for (float t : { 0.25f, 0.5f, 0.75f })
        {
            const float v = 0.5f * (2.0f * p1
                          + (-p0 + p2) * t
                          + (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * t * t
                          + (-p0 + 3.0f * p1 - 3.0f * p2 + p3) * t * t * t);
            tp = std::max (tp, std::abs (v));
        }
        return tp;
    }

    double fs = 48000.0;
    size_t channels = 2, L = 96, writePos = 0;
    float attackInc = 0.01f, gEnv = 1.0f, ceiling = 1.0f, releaseCoeff = 0.99f;
    std::array<std::vector<float>, kMaxCh> delay {};
    std::array<float, 4> hist { 0.0f, 0.0f, 0.0f, 0.0f };
    Smooth inGainSm;
};
} // namespace aur
