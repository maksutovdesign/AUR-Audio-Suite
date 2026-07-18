#pragma once

#include <vector>
#include <cmath>
#include <algorithm>

namespace aur { namespace syn
{
/** Karplus-Strong plucked string: a delay line seeded with a noise burst and
    fed back through a one-pole damping filter. Delay length sets the pitch;
    the damping/decay set tone and sustain. Physical-modelling, very cheap. */
struct KarplusString
{
    void prepare (double sampleRate)
    {
        sr = sampleRate;
        buf.assign ((size_t) (sampleRate / 20.0) + 8, 0.0f);   // down to ~20 Hz
    }

    void setParams (float dampAmt, float decayAmt)
    {
        damp  = std::clamp (dampAmt, 0.0f, 1.0f);
        decay = 0.90f + 0.0999f * std::clamp (decayAmt, 0.0f, 1.0f); // 0.90 … 0.9999
    }

    void pluck (double freq, float brightness, float velocity)
    {
        size = std::clamp ((int) std::round (sr / std::max (20.0, freq)), 2, (int) buf.size() - 1);
        unsigned s = 0x2545F491u ^ (unsigned) size;
        float lp = 0.0f;
        for (int i = 0; i < size; ++i)
        {
            float n = ((float) (s = s * 1664525u + 1013904223u) / 2147483648.0f) - 1.0f;
            lp += brightness * (n - lp);                    // brighter burst = more high end
            buf[(size_t) i] = (brightness * n + (1.0f - brightness) * lp) * velocity;
        }
        idx = 0; active = true; env = velocity; releaseMul = 1.0f;
    }

    void release() { releaseMul = 0.9985f; }   // fast-ish string mute on note-off

    inline float process()
    {
        if (! active) return 0.0f;
        const int nxt = (idx + 1) % size;
        const float cur = buf[(size_t) idx];
        const float avg = 0.5f * (cur + buf[(size_t) nxt]);
        const float filtered = damp * avg + (1.0f - damp) * cur;   // damping = darker
        buf[(size_t) idx] = filtered * decay * releaseMul;
        idx = nxt;
        env = 0.999f * env + 0.001f * std::abs (cur);
        if (env < 1.0e-5f) active = false;
        return cur;
    }

    bool isActive() const { return active; }

    std::vector<float> buf;
    double sr = 44100.0;
    int size = 100, idx = 0;
    float damp = 0.5f, decay = 0.998f, env = 0.0f, releaseMul = 1.0f;
    bool active = false;
};
}} // namespace aur::syn
