#pragma once

#include <cmath>
#include <algorithm>

namespace aur
{
/**
    Noise gate / downward expander. A stereo-linked peak envelope opens the gate
    (gain → 1) above threshold and closes it (gain → floor) after a hold time,
    with independent attack/release. `range` sets how far it closes.
    Realtime-safe, pure C++.
*/
class Gate
{
public:
    void prepare (double sampleRate, int /*ch*/)
    {
        fs = sampleRate;
        env = 0.0f; gain = 1.0f; holdCount = 0;
        setParameters (-40.0f, 60.0f, 1.0f, 50.0f, 100.0f);
    }

    void reset() { env = 0.0f; gain = 1.0f; holdCount = 0; }

    void setParameters (float thresholdDb, float rangeDb, float attackMs, float holdMs, float releaseMs)
    {
        threshold = std::pow (10.0f, thresholdDb / 20.0f);
        floor = std::pow (10.0f, -std::abs (rangeDb) / 20.0f);
        atk = std::exp (-1.0f / (float) (0.001 * std::max (0.1f, attackMs) * fs));
        rel = std::exp (-1.0f / (float) (0.001 * std::max (1.0f, releaseMs) * fs));
        detCoeff = std::exp (-1.0f / (float) (0.001 * 5.0 * fs)); // 5 ms detector
        holdSamples = (int) (0.001 * holdMs * fs);
    }

    void process (float* const* data, int numCh, int numSamples)
    {
        for (int n = 0; n < numSamples; ++n)
        {
            float key = 0.0f;
            for (int c = 0; c < numCh; ++c) key = std::max (key, std::abs (data[c][n]));

            env = key > env ? key : detCoeff * (env - key) + key;

            float targetGain;
            if (env >= threshold) { targetGain = 1.0f; holdCount = holdSamples; }
            else if (holdCount > 0) { targetGain = 1.0f; --holdCount; }
            else targetGain = floor;

            const float coeff = targetGain > gain ? atk : rel;
            gain = coeff * (gain - targetGain) + targetGain;

            for (int c = 0; c < numCh; ++c) data[c][n] *= gain;
        }
        currentGrDb = -20.0f * std::log10 (std::max (gain, 1e-6f));
    }

    float getGainReductionDb() const { return currentGrDb; }

private:
    double fs = 48000.0;
    float threshold = 0.01f, floor = 0.001f, atk = 0.f, rel = 0.f, detCoeff = 0.f;
    float env = 0.0f, gain = 1.0f, currentGrDb = 0.0f;
    int holdSamples = 0, holdCount = 0;
};
} // namespace aur
