#pragma once

#include <cmath>
#include <juce_dsp/juce_dsp.h>

namespace aur
{
/**
    ADAASaturator — the "warm but correct" core of the AUR sound engine.

    Warmth comes from a tanh waveshaper with optional asymmetry (bias), which
    generates the even harmonics associated with tubes/transformers. Correctness
    comes from 1st-order Antiderivative Anti-Aliasing (ADAA): instead of the
    trivial y = f(x) — which folds high harmonics back as inharmonic aliasing —
    we integrate the shaper's antiderivative across the sample interval:

        y[n] = ( F1(x[n]) - F1(x[n-1]) ) / ( x[n] - x[n-1] )

    with a midpoint fallback f((x[n]+x[n-1])/2) when the denominator is tiny.
    For f(x)=tanh(x): F1(x)=log(cosh(x)), which has a closed form — no tables.

    ADAA at base rate already suppresses aliasing comparably to ~2x trivial
    oversampling; this stays realtime-safe (no alloc) and zero-latency. Higher
    oversampling can be layered on top later (v1.1) for extreme drive.

    Three flavours shape the pre/post stages:
      TUBE  — positive bias → even harmonics, warm and round
      TAPE  — near-symmetric, softer knee + gentle HF loss (one-pole)
      IRON  — symmetric, harder drive + low-mid emphasis (transformer punch)
*/
class ADAASaturator
{
public:
    enum class Flavor { Tube, Tape, Iron };

    void prepare (double sampleRate)
    {
        fs = sampleRate;
        driveSm.reset (sampleRate, 0.02);
        mixSm.reset   (sampleRate, 0.02);
        reset();
    }

    void reset()
    {
        for (auto& s : x1)   s = 0.0;
        for (auto& s : F1x1) s = 0.0;
        for (auto& s : lpZ)  s = 0.0f;
        driveSm.setCurrentAndTargetValue (driveSm.getTargetValue());
        mixSm.setCurrentAndTargetValue   (mixSm.getTargetValue());
    }

    void setFlavor (Flavor f) { flavor = f; }

    /** drivePercent 0..100, mixPercent 0..100 */
    void setParameters (float drivePercent, float mixPercent)
    {
        // 0..100% → pre-gain 1x .. ~24x. Musical, generous headroom for "IRON".
        driveSm.setTargetValue (1.0f + (drivePercent / 100.0f) * 23.0f);
        mixSm.setTargetValue   (mixPercent / 100.0f);
    }

    void process (juce::dsp::AudioBlock<float>& block)
    {
        const auto numCh = (int) block.getNumChannels();
        const auto numSamp = (int) block.getNumSamples();

        // Per-flavour static config.
        float bias = 0.0f;      // asymmetry → even harmonics
        float hfLoss = 0.0f;    // one-pole LP amount (tape)
        switch (flavor)
        {
            case Flavor::Tube: bias = 0.30f; hfLoss = 0.0f;  break;
            case Flavor::Tape: bias = 0.06f; hfLoss = 0.35f; break;
            case Flavor::Iron: bias = 0.0f;  hfLoss = 0.0f;  break;
        }

        for (int ch = 0; ch < numCh; ++ch)
        {
            auto* d = block.getChannelPointer ((size_t) ch);
            const int c = juce::jmin (ch, (int) maxCh - 1);

            for (int n = 0; n < numSamp; ++n)
            {
                const float drive = driveSm.getNextValue();
                const float mix   = mixSm.getNextValue();
                const float dry   = d[n];

                // Pre-gain into the nonlinearity, plus bias for asymmetry.
                const double x = (double) dry * drive + (double) bias;

                // ---- ADAA1 ----
                const double F1x = antideriv (x);
                const double diff = x - x1[(size_t) c];
                double y;
                if (std::abs (diff) < 1.0e-6)
                    y = shape (0.5 * (x + x1[(size_t) c]));       // midpoint fallback
                else
                    y = (F1x - F1x1[(size_t) c]) / diff;          // antiderivative slope

                x1[(size_t) c]   = x;
                F1x1[(size_t) c] = F1x;

                // Remove the DC introduced by the bias (shape(bias) is the offset).
                float wet = (float) (y - shape ((double) bias));

                // Level compensation so raising Drive doesn't just get louder.
                wet *= (float) (1.0 / std::tanh ((double) drive));

                // Tape: gentle high-frequency loss (one-pole low-pass).
                if (hfLoss > 0.0f)
                {
                    lpZ[(size_t) c] += hfLoss * (wet - lpZ[(size_t) c]);
                    wet = lpZ[(size_t) c];
                }

                d[n] = dry * (1.0f - mix) + wet * mix;
            }
        }
    }

private:
    // Waveshaper and its first antiderivative (closed form for tanh).
    static inline double shape (double x)     { return std::tanh (x); }
    static inline double antideriv (double x) { return std::log (std::cosh (x)); }

    static constexpr size_t maxCh = 2;
    double fs = 44100.0;
    Flavor flavor = Flavor::Tube;

    double x1[maxCh]   { 0.0, 0.0 };
    double F1x1[maxCh] { 0.0, 0.0 };
    float  lpZ[maxCh]  { 0.0f, 0.0f };

    juce::SmoothedValue<float> driveSm { 1.0f };
    juce::SmoothedValue<float> mixSm   { 1.0f };
};
} // namespace aur
