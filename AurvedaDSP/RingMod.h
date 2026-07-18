#pragma once

#include <cmath>
#include <algorithm>

namespace aur
{
/** Ring modulator — multiplies the signal by a sine carrier.
    A single carrier phase is shared across channels so the modulation stays
    phase-coherent in stereo. Mix blends dry↔ring. Zero latency. */
struct RingMod
{
    void prepare (double sampleRate) { fs = sampleRate; phase = 0.0; }
    void reset()                     { phase = 0.0; }

    void setParameters (float freqHz, float mixPercent)
    {
        freq = freqHz;
        mix  = std::clamp (mixPercent * 0.01f, 0.0f, 1.0f);
    }

    void process (float* const* data, int numChannels, int numSamples)
    {
        constexpr double twoPi = 6.283185307179586476925286766559;
        const double inc = twoPi * (double) freq / fs;

        for (int s = 0; s < numSamples; ++s)
        {
            const float carrier = (float) std::sin (phase);
            phase += inc;
            if (phase >= twoPi) phase -= twoPi;

            const float wet = mix;
            const float dry = 1.0f - mix;
            for (int ch = 0; ch < numChannels; ++ch)
            {
                const float x = data[ch][s];
                data[ch][s] = x * dry + (x * carrier) * wet;
            }
        }
    }

private:
    double fs    = 44100.0;
    double phase = 0.0;
    float  freq  = 440.0f;
    float  mix   = 1.0f;
};
} // namespace aur
