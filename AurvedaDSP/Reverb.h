#pragma once

#include <cmath>
#include <array>
#include <vector>
#include <algorithm>

namespace aur
{
/**
    Warm algorithmic reverb — an 8-line Feedback Delay Network (FDN).

    - Input diffusers (allpass) build echo density fast.
    - 8 mutually-prime delay lines mixed by a normalised Hadamard matrix
      (energy-preserving → smooth, colourless tail).
    - A one-pole low-pass in each feedback path damps highs → the decay grows
      warmer over time rather than metallic.
    - Per-line decay gains derived from an RT60 target keep it unconditionally
      stable (all gains < 1).

    Stereo output is taken from different line subsets for natural width.
    Realtime-safe after prepare(); pure C++ (offline-testable).
*/
class Reverb
{
public:
    static constexpr int N = 8;

    void prepare (double sampleRate, int /*numChannels*/)
    {
        fs = sampleRate;

        const double baseMs[N] = { 23.0, 29.0, 31.0, 37.0, 41.0, 43.0, 47.0, 53.0 };
        for (size_t i = 0; i < (size_t) N; ++i)
        {
            baseLen[i] = baseMs[i];
            lines[i].assign ((size_t) (baseMs[i] * 0.001 * fs * 2.0) + 4, 0.0f);
        }
        const double diffMs[4] = { 8.0, 11.0, 13.0, 17.0 };
        for (size_t i = 0; i < 4; ++i)
        {
            diff[i].assign ((size_t) (diffMs[i] * 0.001 * fs) + 4, 0.0f);
            diffLen[i] = (int) (diffMs[i] * 0.001 * fs);
        }
        preBuf.assign ((size_t) (0.2 * fs) + 4, 0.0f);

        setParameters (0.5f, 2.5f, 0.4f, 20.0f, 30.0f);
        reset();
    }

    void reset()
    {
        for (auto& l : lines) std::fill (l.begin(), l.end(), 0.0f);
        for (auto& d : diff)  std::fill (d.begin(), d.end(), 0.0f);
        std::fill (preBuf.begin(), preBuf.end(), 0.0f);
        for (size_t i = 0; i < (size_t) N; ++i) { widx[i] = 0; lp[i] = 0.0f; }
        for (size_t i = 0; i < 4; ++i) didx[i] = 0;
        preIdx = 0;
    }

    void setParameters (float size, float decaySec, float damp, float preDelayMs, float /*width*/)
    {
        const double scale = 0.5 + (double) size;
        for (size_t i = 0; i < (size_t) N; ++i)
        {
            int len = (int) (baseLen[i] * 0.001 * fs * scale);
            len = std::max (1, std::min (len, (int) lines[i].size() - 1));
            delayLen[i] = len;
            const double rt = std::max (0.1f, decaySec);
            g[i] = (float) std::pow (10.0, -3.0 * (double) len / (rt * fs));
        }
        dampCoeff = 0.05f + 0.9f * damp;
        preSamples = std::min ((int) (preDelayMs * 0.001 * fs), (int) preBuf.size() - 1);
    }

    void process (float* left, float* right, int numSamples)
    {
        const int preSize = (int) preBuf.size();

        for (int n = 0; n < numSamples; ++n)
        {
            float in = 0.5f * (left[n] + right[n]);

            preBuf[(size_t) preIdx] = in;
            const int pr = (preIdx - preSamples + preSize) % preSize;
            in = preBuf[(size_t) pr];
            preIdx = (preIdx + 1) % preSize;

            for (size_t i = 0; i < 4; ++i)
                in = allpass (diff[i], didx[i], diffLen[i], in, 0.6f);

            float v[N];
            for (size_t i = 0; i < (size_t) N; ++i)
            {
                const int sz = (int) lines[i].size();
                const int rd = (widx[i] - delayLen[i] + sz) % sz;
                const float s = lines[i][(size_t) rd];
                lp[i] += dampCoeff * (s - lp[i]);
                v[i] = lp[i] * g[i];
            }

            hadamard (v);

            for (size_t i = 0; i < (size_t) N; ++i)
            {
                const float sign = (i & 1) ? -1.0f : 1.0f;
                lines[i][(size_t) widx[i]] = in * sign + v[i];
                widx[i] = (widx[i] + 1) % (int) lines[i].size();
            }

            left[n]  = 0.5f * (v[0] + v[2] + v[4] + v[6]);
            right[n] = 0.5f * (v[1] + v[3] + v[5] + v[7]);
        }
    }

private:
    static float allpass (std::vector<float>& buf, int& idx, int len, float x, float k)
    {
        const int sz = (int) buf.size();
        const int rd = (idx - len + sz) % sz;
        const float d = buf[(size_t) rd];
        const float y = -k * x + d;
        buf[(size_t) idx] = x + k * y;
        idx = (idx + 1) % sz;
        return y;
    }

    static void hadamard (float* a)
    {
        for (int len = 1; len < N; len <<= 1)
            for (int i = 0; i < N; i += (len << 1))
                for (int j = i; j < i + len; ++j)
                {
                    const float x = a[j], y = a[j + len];
                    a[j] = x + y; a[j + len] = x - y;
                }
        const float norm = 1.0f / std::sqrt ((float) N);
        for (int i = 0; i < N; ++i) a[i] *= norm;
    }

    double fs = 48000.0;
    std::array<std::vector<float>, N> lines {};
    std::array<std::vector<float>, 4> diff {};
    std::vector<float> preBuf;

    std::array<double, N> baseLen {};
    std::array<int, N>    delayLen {}, widx {};
    std::array<float, N>  g {}, lp {};
    std::array<int, 4>    diffLen {}, didx {};
    int preIdx = 0, preSamples = 0;
    float dampCoeff = 0.5f;
};
} // namespace aur
