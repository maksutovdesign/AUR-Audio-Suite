#pragma once

#include <cmath>
#include <array>
#include <vector>
#include <algorithm>
#include "Biquad.h"

namespace aur
{
/**
    ITU-R BS.1770 / EBU R128 loudness meter. Applies the two-stage K-weighting
    filter (high-shelf + high-pass) per channel, sums channel energies, and
    integrates over sliding windows: 400 ms (Momentary) and 3 s (Short-term).
    Returns LUFS. Pure C++ (uses aur::Biquad). Not realtime-critical — used for
    the meter display; call push() per sample from the audio thread (no alloc).
*/
class LoudnessMeter
{
public:
    static constexpr size_t kMaxCh = 2;

    void prepare (double sampleRate, int numChannels)
    {
        fs = sampleRate;
        channels = (numChannels < (int) kMaxCh) ? (size_t) numChannels : kMaxCh;

        for (size_t c = 0; c < kMaxCh; ++c)
        {
            shelf[c].setHighShelf (fs, 1681.97, 0.7071, 3.9999);
            hp[c].setHighpass     (fs, 38.135, 0.5);
            shelf[c].reset();
            hp[c].reset();
        }

        momLen = (size_t) (0.400 * fs);
        stLen  = (size_t) (3.000 * fs);
        ring.assign (stLen, 0.0);   // ring holds summed K-weighted power
        writePos = 0;
        momSum = 0.0; stSum = 0.0;
        filled = 0;
    }

    void reset()
    {
        std::fill (ring.begin(), ring.end(), 0.0);
        writePos = 0; momSum = 0.0; stSum = 0.0; filled = 0;
        for (size_t c = 0; c < kMaxCh; ++c) { shelf[c].reset(); hp[c].reset(); }
    }

    /** Push one interleaved-by-pointer frame: samples[c]. */
    void push (const float* const* samples, size_t numCh)
    {
        numCh = numCh < channels ? numCh : channels;

        double power = 0.0;
        for (size_t c = 0; c < numCh; ++c)
        {
            const float k = hp[c].process (shelf[c].process (samples[c][0]));
            power += (double) k * (double) k;
        }

        // Slide the 3 s ring; maintain running sums for 3 s and 0.4 s windows.
        const double old = ring[writePos];
        ring[writePos] = power;

        stSum += power - old;

        // Momentary window is the most-recent momLen samples of the ring.
        momSum += power;
        const size_t momOldPos = (writePos + stLen - momLen) % stLen;
        momSum -= ring[momOldPos];

        writePos = (writePos + 1) % stLen;
        if (filled < stLen) ++filled;
    }

    float momentaryLufs() const { return lufs (momSum, std::min (filled, momLen)); }
    float shortTermLufs() const { return lufs (stSum,  filled); }

private:
    float lufs (double sumPower, size_t n) const
    {
        if (n == 0) return -100.0f;
        const double mean = sumPower / (double) n;
        if (mean <= 1.0e-12) return -100.0f;
        return (float) (-0.691 + 10.0 * std::log10 (mean));
    }

    double fs = 48000.0;
    size_t channels = 2;
    std::array<Biquad, kMaxCh> shelf {}, hp {};

    std::vector<double> ring;
    size_t momLen = 0, stLen = 0, writePos = 0, filled = 0;
    double momSum = 0.0, stSum = 0.0;
};
} // namespace aur
