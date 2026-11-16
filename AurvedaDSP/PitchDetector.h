#pragma once

#include <cmath>
#include <array>
#include <atomic>
#include <algorithm>
#include <cstdint>

namespace aur
{
/**
    Monophonic pitch detector. Feed mono samples on the audio thread with pushMono(): it only
    keeps the latest kWin samples. getFrequency() works the pitch out from them when it is
    called (from the UI, a few dozen times a second), so the audio thread does no analysis.

    The method is a normalised square difference (McLeod's NSDF): the first strong peak after
    the curve first goes negative, refined with a parabola. The earlier version took the largest
    plain autocorrelation value, which read low notes wrongly (a guitar's low E, 82 Hz, as
    1480 Hz) and ran inside the audio callback. Range 40 Hz to 2 kHz; 0 for silence or noise.
*/
class PitchDetector
{
public:
    static constexpr int kWin = 4096;

    void prepare (double sampleRate)
    {
        fs = sampleRate;
        ring.fill (0.0f);
        writePos.store (0, std::memory_order_relaxed);
    }

    void pushMono (float x) noexcept
    {
        const auto w = writePos.load (std::memory_order_relaxed);
        ring[w & (kWin - 1)] = x;
        writePos.store (w + 1, std::memory_order_release);
    }

    /** The pitch of the latest kWin samples, in Hz (0: none). Call it off the audio thread. */
    float getFrequency() const
    {
        std::array<float, kWin> w;
        const auto end = writePos.load (std::memory_order_acquire);
        for (uint32_t i = 0; i < (uint32_t) kWin; ++i)
            w[i] = ring[(end - (uint32_t) kWin + i) & (kWin - 1)];
        return detect (w.data(), kWin, fs);
    }

    /** The same analysis on any window of samples. */
    static float detect (const float* w, int n, double sampleRate)
    {
        double energy = 0.0;
        for (int i = 0; i < n; ++i)
            energy += (double) w[i] * w[i];
        if (n <= 0 || std::sqrt (energy / n) < 1.0e-4)
            return 0.0f; // silence
        const int minLag = std::max (2, (int) (sampleRate / 2000.0));
        const int maxLag = std::min (n / 2, (int) (sampleRate / 40.0));
        std::array<float, kWin / 2 + 2> nsdf {};
        double head = energy, tail = energy; // energy of w[0, n - lag) and of w[lag, n)
        for (int lag = 1; lag <= maxLag + 1 && lag < n && lag < (int) nsdf.size(); ++lag)
        {
            head -= (double) w[n - lag] * w[n - lag];
            tail -= (double) w[lag - 1] * w[lag - 1];
            double ac = 0.0;
            for (int i = 0; i + lag < n; ++i)
                ac += (double) w[i] * w[i + lag];
            nsdf[(size_t) lag] = (float) (2.0 * ac / (head + tail + 1.0e-12));
        }
        int from = 1; // past the peak at lag 0: to where the curve first goes negative
        while (from <= maxLag && nsdf[(size_t) from] > 0.0f)
            ++from;
        from = std::max (from, minLag);
        float best = 0.0f;
        for (int lag = from; lag <= maxLag; ++lag)
            best = std::max (best, nsdf[(size_t) lag]);
        if (best < 0.6f)
            return 0.0f; // noise: no clear period
        for (int lag = from; lag <= maxLag; ++lag)
        {
            const float y0 = nsdf[(size_t) (lag - 1)], y1 = nsdf[(size_t) lag], y2 = nsdf[(size_t) (lag + 1)];
            if (y1 >= 0.9f * best && y1 >= y0 && y1 > y2)
            {
                const float curve = y0 - 2.0f * y1 + y2;
                const float shift = std::abs (curve) > 1.0e-9f ? 0.5f * (y0 - y2) / curve : 0.0f;
                return (float) (sampleRate / ((double) lag + shift));
            }
        }
        return 0.0f;
    }

private:
    double fs = 48000.0;
    std::array<float, kWin> ring {};
    std::atomic<uint32_t> writePos { 0 };
};
} // namespace aur
