#pragma once

#include <cmath>
#include <algorithm>

namespace aur
{
/**
    Transient shaper. Two stereo-linked envelope followers (fast + slow); their
    ratio tells attack (fast>slow) from sustain (fast<slow). Attack and Sustain
    controls raise or lower each phase. Realtime-safe, pure C++.
*/
class TransientShaper
{
public:
    void prepare (double sampleRate)
    {
        fs = sampleRate;
        fAtk = std::exp (-1.0f / (float) (0.0005 * fs));
        fRel = std::exp (-1.0f / (float) (0.050  * fs));
        sAtk = std::exp (-1.0f / (float) (0.020  * fs));
        sRel = std::exp (-1.0f / (float) (0.080  * fs));
        fast = slow = 0.0f;
    }
    void reset() { fast = slow = 0.0f; }

    /** attackPct/sustainPct in -100..+100. */
    void setParameters (float attackPct, float sustainPct)
    {
        atkAmt = attackPct / 100.0f;
        susAmt = sustainPct / 100.0f;
    }

    void process (float* const* data, int numCh, int numSamples)
    {
        for (int n = 0; n < numSamples; ++n)
        {
            float key = 0.0f;
            for (int c = 0; c < numCh; ++c) key = std::max (key, std::abs (data[c][n]));

            fast = key > fast ? fAtk * (fast - key) + key : fRel * (fast - key) + key;
            slow = key > slow ? sAtk * (slow - key) + key : sRel * (slow - key) + key;

            const float ratio = slow > 1e-6f ? fast / slow : 1.0f;
            float g = ratio >= 1.0f ? std::pow (ratio, atkAmt) : std::pow (ratio, -susAmt);
            g = std::min (4.0f, std::max (0.25f, g));

            for (int c = 0; c < numCh; ++c) data[c][n] *= g;
        }
    }

private:
    double fs = 48000.0;
    float fast = 0.0f, slow = 0.0f, fAtk = 0.f, fRel = 0.f, sAtk = 0.f, sRel = 0.f;
    float atkAmt = 0.0f, susAmt = 0.0f;
};
} // namespace aur
