#pragma once

#include <cmath>
#include <array>
#include "Biquad.h"

namespace aur
{
/**
    Six-stage parametric EQ: HPF → Low-shelf → Bell → High-shelf → LPF.
    Clean minimum-phase RBJ biquads. Exposes a combined magnitude response for
    drawing the EQ curve. Realtime-safe (in-place coeff updates, no allocation).
*/
class ParametricEQ
{
public:
    static constexpr size_t kMaxCh = 2;

    enum Band { HP = 0, LowShelf, Bell, HighShelf, LP, kNumBands };

    void prepare (double sampleRate, int numChannels)
    {
        fs = sampleRate;
        channels = (numChannels < (int) kMaxCh) ? (size_t) numChannels : kMaxCh;
        reset();
    }

    void reset()
    {
        for (size_t b = 0; b < kNumBands; ++b)
            for (size_t c = 0; c < kMaxCh; ++c)
                proto[b][c].reset();
    }

    void setParameters (float hpFreq, float lsFreq, float lsGain,
                        float bellFreq, float bellGain, float bellQ,
                        float hsFreq, float hsGain, float lpFreq)
    {
        hpOn = hpFreq > 21.0f;
        lpOn = lpFreq < 19900.0f;

        design (Band::HP,       [&] (Biquad& f) { f.setHighpass  (fs, hpFreq, 0.707); });
        design (Band::LowShelf, [&] (Biquad& f) { f.setLowShelf  (fs, lsFreq, 0.707, lsGain); });
        design (Band::Bell,     [&] (Biquad& f) { f.setPeaking   (fs, bellFreq, bellQ, bellGain); });
        design (Band::HighShelf,[&] (Biquad& f) { f.setHighShelf (fs, hsFreq, 0.707, hsGain); });
        design (Band::LP,       [&] (Biquad& f) { f.setLowpass   (fs, lpFreq, 0.707); });

        active[Band::HP] = hpOn;
        active[Band::LP] = lpOn;
        active[Band::LowShelf]  = std::abs (lsGain) > 0.05f;
        active[Band::HighShelf] = std::abs (hsGain) > 0.05f;
        active[Band::Bell]      = std::abs (bellGain) > 0.05f;
    }

    void process (float* const* data, int numChIn, int numSamples)
    {
        const size_t nc = ((size_t) numChIn < channels) ? (size_t) numChIn : channels;
        for (int n = 0; n < numSamples; ++n)
            for (size_t c = 0; c < nc; ++c)
            {
                float x = data[c][n];
                for (size_t b = 0; b < kNumBands; ++b)
                    if (active[b]) x = proto[b][c].process (x);
                data[c][n] = x;
            }
    }

    /** Combined response in dB at a given frequency (for the UI curve). */
    float responseDb (double freq) const
    {
        const double w = 2.0 * M_PI * freq / fs;
        double mag = 1.0;
        for (size_t b = 0; b < kNumBands; ++b)
            if (active[b]) mag *= proto[b][0].magnitudeAt (w);
        return (float) (20.0 * std::log10 (mag + 1.0e-9));
    }

private:
    template <typename Fn>
    void design (int band, Fn&& fn)
    {
        for (size_t c = 0; c < kMaxCh; ++c)
        {
            // Configure a prototype on channel 0, copy coefficients to the rest
            // (keeps state per channel but coefficients identical).
            fn (proto[(size_t) band][c]);
        }
    }

    double fs = 48000.0;
    size_t channels = 2;
    std::array<std::array<Biquad, kMaxCh>, kNumBands> proto {};
    std::array<bool, kNumBands> active { true, true, true, true, true };
    bool hpOn = false, lpOn = false;
};
} // namespace aur
