#pragma once

#include <array>
#include <juce_dsp/juce_dsp.h>
#include "Biquad.h"
#include "Compressor.h"

namespace aur
{
/**
    3-band compressor. Linkwitz-Riley 4th-order crossovers (two cascaded
    Butterworth biquads per split) divide the signal into low/mid/high; each
    band has its own compressor; the bands are summed back. Per-band threshold
    and ratio; shared attack/release. Realtime-safe after prepare().
*/
class MultibandComp
{
public:
    static constexpr int NB = 3;
    static constexpr size_t kMaxCh = 2;

    void prepare (double sampleRate, int numChannels, int maxBlock)
    {
        fs = sampleRate;
        channels = (numChannels < (int) kMaxCh) ? (size_t) numChannels : kMaxCh;
        juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) maxBlock, (juce::uint32) numChannels };
        for (auto& c : comp) c.prepare (spec);
        for (auto& b : band) b.setSize ((int) kMaxCh, maxBlock);
        setParameters (200.0f, 2500.0f, -18.f, 3.f, -18.f, 3.f, -18.f, 3.f);
    }

    void setParameters (float xLo, float xHi,
                        float thrL, float ratL, float thrM, float ratM, float thrH, float ratH)
    {
        for (size_t c = 0; c < kMaxCh; ++c)
        {
            lp1a[c].setLowpass  (fs, xLo, 0.7071); lp1b[c].setLowpass  (fs, xLo, 0.7071);
            hp1a[c].setHighpass (fs, xLo, 0.7071); hp1b[c].setHighpass (fs, xLo, 0.7071);
            lp2a[c].setLowpass  (fs, xHi, 0.7071); lp2b[c].setLowpass  (fs, xHi, 0.7071);
            hp2a[c].setHighpass (fs, xHi, 0.7071); hp2b[c].setHighpass (fs, xHi, 0.7071);
        }
        comp[0].setParameters (thrL, ratL, 15.f, 150.f, 0.f, 100.f);
        comp[1].setParameters (thrM, ratM, 15.f, 150.f, 0.f, 100.f);
        comp[2].setParameters (thrH, ratH, 15.f, 150.f, 0.f, 100.f);
    }

    void process (float* const* data, int numCh, int numSamples)
    {
        const size_t nc = ((size_t) numCh < channels) ? (size_t) numCh : channels;

        for (size_t c = 0; c < nc; ++c)
        {
            auto* lo = band[0].getWritePointer ((int) c);
            auto* md = band[1].getWritePointer ((int) c);
            auto* hi = band[2].getWritePointer ((int) c);
            for (int i = 0; i < numSamples; ++i)
            {
                const float x = data[c][i];
                const float low  = lp1b[c].process (lp1a[c].process (x));
                const float hpf  = hp1b[c].process (hp1a[c].process (x));
                const float mid  = lp2b[c].process (lp2a[c].process (hpf));
                const float high = hp2b[c].process (hp2a[c].process (hpf));
                lo[i] = low; md[i] = mid; hi[i] = high;
            }
        }

        for (int b = 0; b < NB; ++b)
        {
            juce::dsp::AudioBlock<float> blk (band[(size_t) b]);
            auto sub = blk.getSubsetChannelBlock (0, nc).getSubBlock (0, (size_t) numSamples);
            comp[(size_t) b].process (sub);
        }

        for (size_t c = 0; c < nc; ++c)
        {
            const auto* lo = band[0].getReadPointer ((int) c);
            const auto* md = band[1].getReadPointer ((int) c);
            const auto* hi = band[2].getReadPointer ((int) c);
            for (int i = 0; i < numSamples; ++i) data[c][i] = lo[i] + md[i] + hi[i];
        }
    }

    float getGainReductionDb() const
    {
        float g = 0.0f; for (auto& c : comp) g = std::max (g, c.getGainReductionDb()); return g;
    }

private:
    double fs = 48000.0;
    size_t channels = 2;
    std::array<Compressor, NB> comp;
    std::array<juce::AudioBuffer<float>, NB> band;
    std::array<Biquad, kMaxCh> lp1a, lp1b, hp1a, hp1b, lp2a, lp2b, hp2a, hp2b;
};
} // namespace aur
