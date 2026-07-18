#pragma once

#include <cmath>
#include <algorithm>

namespace aur { namespace syn
{
/** Zero-delay-feedback (TPT) state-variable filter — Cytomic/Zavalishin form.
    Per-sample, self-oscillating, cheap. Gives LP/BP/HP from one core. Ideal for
    per-voice synth filtering (no protected-method issues like JUCE's ladder). */
struct SvfZDF
{
    void prepare (double sampleRate) { sr = sampleRate; reset(); }
    void reset() { ic1 = ic2 = 0.0f; }

    /** cutoff in Hz, res in 0..1 (→ Q ≈ 0.5 … ~20, self-osc near 1). */
    void set (float cutoffHz, float res)
    {
        cutoffHz = std::clamp (cutoffHz, 20.0f, (float) (sr * 0.49));
        g = (float) std::tan (3.141592653589793 * cutoffHz / sr);
        const float Q = 0.5f + res * res * 24.0f;
        k = 1.0f / Q;
        const float d = 1.0f / (1.0f + g * (g + k));
        a1 = d; a2 = g * d; a3 = g * a2;
    }

    /** Process one sample; `mode` 0=LP 1=BP 2=HP. */
    inline float process (float v0, int mode)
    {
        const float v3 = v0 - ic2;
        const float v1 = a1 * ic1 + a2 * v3;
        const float v2 = ic2 + a2 * ic1 + a3 * v3;
        ic1 = 2.0f * v1 - ic1;
        ic2 = 2.0f * v2 - ic2;
        if (mode == 1) return v1;                 // band
        if (mode == 2) return v0 - k * v1 - v2;   // high
        return v2;                                // low
    }

    double sr = 44100.0;
    float g = 0, k = 1, a1 = 0, a2 = 0, a3 = 0, ic1 = 0, ic2 = 0;
};
}} // namespace aur::syn
