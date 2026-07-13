#pragma once

#include <vector>
#include <algorithm>

namespace aur
{
/** Mono fractional delay line with linear interpolation (for modulated delays). */
struct FractionalDelay
{
    std::vector<float> buf;
    int widx = 0;

    void prepare (double fs, double maxMs)
    {
        buf.assign ((size_t) (maxMs * 0.001 * fs) + 4, 0.0f);
        widx = 0;
    }
    void reset() { std::fill (buf.begin(), buf.end(), 0.0f); widx = 0; }

    void write (float x) { buf[(size_t) widx] = x; widx = (widx + 1) % (int) buf.size(); }

    /** Read `delaySamples` (fractional) behind the write head. */
    float read (float delaySamples) const
    {
        const int sz = (int) buf.size();
        // Clamp so we always read behind the write head (never negative delay).
        if (delaySamples < 1.0f) delaySamples = 1.0f;
        if (delaySamples > (float) (sz - 2)) delaySamples = (float) (sz - 2);
        float rp = (float) widx - 1.0f - delaySamples;
        while (rp < 0.0f) rp += (float) sz;
        const int i0 = (int) rp;
        const float frac = rp - (float) i0;
        const int i1 = (i0 + 1) % sz;
        return buf[(size_t) i0] * (1.0f - frac) + buf[(size_t) i1] * frac;
    }
};
} // namespace aur
