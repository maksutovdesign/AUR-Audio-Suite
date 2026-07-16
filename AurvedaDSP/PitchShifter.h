#pragma once

#include <cmath>
#include <array>
#include <vector>
#include <algorithm>

namespace aur
{
/**
    Real-time pitch shifter — classic two-tap delay-line method. A delay line is
    read at the pitch ratio; two read taps a half-grain apart are cross-faded by
    a raised-cosine window so the buffer wrap is inaudible. Time-domain, low
    latency, robust (a phase-vocoder version can come later). Per-channel state,
    shared grain phase for stereo coherence. Realtime-safe after prepare().
*/
class PitchShifter
{
public:
    void prepare (double sampleRate, int /*ch*/)
    {
        fs = sampleRate;
        grain = 2048;
        for (auto& r : ring) r.assign ((size_t) (grain * 2 + 4), 0.0f);
        for (auto& w : widx) w = 0;
        phase = 0.0f;
        setParameters (0.0f, 100.0f);
    }
    void reset() { for (auto& r : ring) std::fill (r.begin(), r.end(), 0.0f); for (auto& w : widx) w = 0; phase = 0.0f; }

    /** semitones -24..+24, mix 0..100. */
    void setParameters (float semitones, float mixPct)
    {
        const float ratio = std::pow (2.0f, semitones / 12.0f);
        inc = (1.0f - ratio) / (float) grain;   // read-pointer drift per sample
        mix = mixPct / 100.0f;
    }

    void process (float* const* data, int numCh, int numSamples)
    {
        const int nc = numCh < 2 ? numCh : 2;
        const int sz = (int) ring[0].size();
        for (int i = 0; i < numSamples; ++i)
        {
            phase += inc;
            if (phase >= 1.0f) phase -= 1.0f;
            if (phase < 0.0f)  phase += 1.0f;
            const float phase2 = phase >= 0.5f ? phase - 0.5f : phase + 0.5f;
            const float d1 = phase  * (float) grain;
            const float d2 = phase2 * (float) grain;
            const float w1 = 0.5f - 0.5f * std::cos (2.0f * (float) M_PI * phase);
            const float w2 = 0.5f - 0.5f * std::cos (2.0f * (float) M_PI * phase2);

            for (int c = 0; c < nc; ++c)
            {
                ring[(size_t) c][(size_t) widx[c]] = data[c][i];
                const float wet = fracRead (ring[(size_t) c], widx[c], d1, sz) * w1
                                + fracRead (ring[(size_t) c], widx[c], d2, sz) * w2;
                data[c][i] = data[c][i] * (1.0f - mix) + wet * mix;
                widx[c] = (widx[c] + 1) % sz;
            }
        }
    }

private:
    static float fracRead (const std::vector<float>& buf, int widx, float delay, int sz)
    {
        float rp = (float) widx - delay;
        while (rp < 0.0f) rp += (float) sz;
        const int i0 = (int) rp;
        const float f = rp - (float) i0;
        const int i1 = (i0 + 1) % sz;
        return buf[(size_t) i0] * (1.0f - f) + buf[(size_t) i1] * f;
    }

    double fs = 48000.0;
    int grain = 2048;
    std::array<std::vector<float>, 2> ring;
    std::array<int, 2> widx { 0, 0 };
    float phase = 0.0f, inc = 0.0f, mix = 1.0f;
};
} // namespace aur
