#pragma once

#include <cmath>
#include <vector>
#include <algorithm>

namespace aur
{
/**
    Stereo delay with damped feedback, ping-pong and stereo width. The feedback
    path runs through a one-pole low-pass so repeats get darker (analog-ish tape
    echo) rather than harsh. Realtime-safe after prepare().
*/
class Delay
{
public:
    void prepare (double sampleRate, double maxSeconds = 2.0)
    {
        fs = sampleRate;
        const int maxLen = (int) (maxSeconds * fs) + 4;
        bufL.assign ((size_t) maxLen, 0.0f);
        bufR.assign ((size_t) maxLen, 0.0f);
        reset();
        setParameters (350.0f, 35.0f, 40.0f, 30.0f, false, 100.0f);
    }

    void reset()
    {
        std::fill (bufL.begin(), bufL.end(), 0.0f);
        std::fill (bufR.begin(), bufR.end(), 0.0f);
        widx = 0; lpL = lpR = 0.0f;
    }

    void setParameters (float timeMs, float feedbackPct, float dampPct,
                        float mixPct, bool pingpong, float widthPct)
    {
        int d = (int) (timeMs * 0.001f * fs);
        delaySamples = std::max (1, std::min (d, (int) bufL.size() - 1));
        fb    = std::min (0.98f, feedbackPct / 100.0f);   // < 1 → stable
        damp  = 0.05f + 0.9f * (dampPct / 100.0f);
        mix   = mixPct / 100.0f;
        ping  = pingpong;
        width = widthPct / 100.0f;
    }

    void process (float* left, float* right, int numSamples)
    {
        const int sz = (int) bufL.size();
        for (int n = 0; n < numSamples; ++n)
        {
            const int rd = (widx - delaySamples + sz) % sz;
            float dL = bufL[(size_t) rd];
            float dR = bufR[(size_t) rd];

            lpL += damp * (dL - lpL);
            lpR += damp * (dR - lpR);

            const float inL = left[n], inR = right[n];
            if (ping)
            {
                bufL[(size_t) widx] = inL + lpR * fb;
                bufR[(size_t) widx] = inR + lpL * fb;
            }
            else
            {
                bufL[(size_t) widx] = inL + lpL * fb;
                bufR[(size_t) widx] = inR + lpR * fb;
            }

            float wetL = dL, wetR = dR;
            const float mid  = 0.5f * (wetL + wetR);
            const float side = 0.5f * (wetL - wetR) * width;
            wetL = mid + side; wetR = mid - side;

            left[n]  = inL * (1.0f - mix) + wetL * mix;
            right[n] = inR * (1.0f - mix) + wetR * mix;
            widx = (widx + 1) % sz;
        }
    }

private:
    double fs = 48000.0;
    std::vector<float> bufL, bufR;
    int widx = 0, delaySamples = 1;
    float fb = 0.3f, damp = 0.5f, mix = 0.3f, width = 1.0f;
    float lpL = 0.0f, lpR = 0.0f;
    bool ping = false;
};
} // namespace aur
