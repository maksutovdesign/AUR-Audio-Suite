#pragma once
#include <vector>
#include <array>
#include <cmath>
#include <memory>

namespace aur { namespace syn
{
/** Morphing band-limited wavetable oscillator.
    4 base shapes (sine → triangle → saw → square), each pre-rendered
    additively at 9 mip levels (halving harmonic count) so any pitch stays
    alias-free. `setMorph` crossfades between adjacent shapes. */
struct WavetableSet
{
    static constexpr int N = 2048, MIPS = 9, SHAPES = 4;
    std::vector<float> data;   // [shape][mip][N]

    WavetableSet()
    {
        data.assign ((size_t) SHAPES * MIPS * N, 0.0f);
        for (int sh = 0; sh < SHAPES; ++sh)
            for (int m = 0; m < MIPS; ++m)
            {
                const int maxH = std::max (1, 256 >> m);
                float* t = &data[(size_t) (sh * MIPS + m) * N];
                double peak = 0.0;
                for (int h = 1; h <= maxH; ++h)
                {
                    double a = 0.0;
                    switch (sh)
                    {
                        case 0: a = h == 1 ? 1.0 : 0.0; break;                       // sine
                        case 1: a = (h % 2) ? std::pow (-1.0, (h-1)/2) / (double) (h*h) : 0.0; break; // tri
                        case 2: a = 1.0 / h; break;                                  // saw
                        case 3: a = (h % 2) ? 1.0 / h : 0.0; break;                  // square
                    }
                    if (a == 0.0) continue;
                    for (int i = 0; i < N; ++i)
                        t[i] += (float) (a * std::sin (6.283185307179586 * h * i / (double) N));
                }
                for (int i = 0; i < N; ++i) peak = std::max (peak, (double) std::abs (t[i]));
                if (peak > 0.0) for (int i = 0; i < N; ++i) t[i] = (float) (t[i] / peak);
            }
    }

    static const WavetableSet& instance() { static WavetableSet s; return s; }
};

struct WavetableOsc
{
    void prepare (double sampleRate) { sr = sampleRate; }
    void reset (double ph = 0.0) { phase = ph; }
    void setFrequency (double hz) { inc = hz / sr; }
    void setMorph (float m01) { morph = std::min (std::max (m01, 0.0f), 1.0f) * (WavetableSet::SHAPES - 1); }

    inline float next()
    {
        const auto& ws = WavetableSet::instance();
        // Mip from increment: harmonics allowed ≈ 0.5/inc; 256>>m ≤ that.
        int mip = 0; double h = 0.5 / std::max (1.0e-6, inc);
        while (mip < WavetableSet::MIPS - 1 && (256 >> mip) > h) ++mip;

        const int s0 = (int) morph;
        const int s1 = std::min (s0 + 1, WavetableSet::SHAPES - 1);
        const float fs = morph - (float) s0;

        const double p = phase * WavetableSet::N;
        const int i0 = (int) p & (WavetableSet::N - 1);
        const int i1 = (i0 + 1) & (WavetableSet::N - 1);
        const float fr = (float) (p - std::floor (p));

        auto samp = [&] (int shape)
        {
            const float* t = &ws.data[(size_t) (shape * WavetableSet::MIPS + mip) * WavetableSet::N];
            return t[i0] + fr * (t[i1] - t[i0]);
        };
        const float v = samp (s0) * (1.0f - fs) + samp (s1) * fs;

        phase += inc; if (phase >= 1.0) phase -= 1.0;
        return v;
    }

    double sr = 44100, phase = 0, inc = 0;
    float morph = 2.0f;
};
}} // namespace aur::syn
