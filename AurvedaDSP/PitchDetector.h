#pragma once

#include <cmath>
#include <array>
#include <atomic>

namespace aur
{
/**
    Monophonic pitch detector via normalised autocorrelation. Feed mono samples;
    it re-estimates the fundamental every `kWin` samples and publishes the
    frequency (Hz) atomically for the UI. Realtime-safe (fixed buffers).
*/
class PitchDetector
{
public:
    static constexpr int kWin = 2048;

    void prepare (double sampleRate)
    {
        fs = sampleRate;
        minLag = (int) (fs / 1500.0); if (minLag < 2) minLag = 2;
        maxLag = (int) (fs / 50.0);   if (maxLag > kWin - 1) maxLag = kWin - 1;
        buf.fill (0.0f); pos = 0; filled = 0;
    }

    void pushMono (float x)
    {
        buf[(size_t) pos] = x;
        pos = (pos + 1) % kWin;
        if (++filled >= kWin) { filled = 0; detect(); }
    }

    float getFrequency() const { return freq.load (std::memory_order_relaxed); }

private:
    void detect()
    {
        // Linearise the ring (oldest first) into a work array.
        static thread_local std::array<float, kWin> w;
        float rms = 0.0f;
        for (int i = 0; i < kWin; ++i) { const float v = buf[(size_t) ((pos + i) % kWin)]; w[(size_t) i] = v; rms += v * v; }
        rms = std::sqrt (rms / kWin);
        if (rms < 1.0e-4f) { freq.store (0.0f, std::memory_order_relaxed); return; } // silence

        double bestVal = 0.0; int bestLag = 0;
        double norm0 = 1e-9; for (int i = 0; i < kWin; ++i) norm0 += (double) w[(size_t) i] * w[(size_t) i];
        for (int lag = minLag; lag <= maxLag; ++lag)
        {
            double ac = 0.0;
            for (int i = 0; i + lag < kWin; ++i) ac += (double) w[(size_t) i] * w[(size_t) (i + lag)];
            const double nac = ac / norm0;
            if (nac > bestVal) { bestVal = nac; bestLag = lag; }
        }
        if (bestLag > 0 && bestVal > 0.3)
            freq.store ((float) (fs / bestLag), std::memory_order_relaxed);
        else
            freq.store (0.0f, std::memory_order_relaxed);
    }

    double fs = 48000.0;
    int minLag = 32, maxLag = 960, pos = 0, filled = 0;
    std::array<float, kWin> buf {};
    std::atomic<float> freq { 0.0f };
};
} // namespace aur
