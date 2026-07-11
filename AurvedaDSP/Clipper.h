#pragma once

#include <cmath>

namespace aur
{
/**
    Anti-aliased clipper. Hard-clip uses 1st-order ADAA (the hard-clip
    antiderivative is closed-form and piecewise), so it stays clean where a
    trivial clipper would alias badly. Soft mode uses tanh. Drive in, ceiling,
    dry/wet mix, output compensation.
*/
class Clipper
{
public:
    enum class Mode { Hard, Soft };

    void prepare (double /*fs*/) { for (auto& s : x1) s = 0.0; for (auto& s : F1x1) s = 0.0; }
    void reset()                 { for (auto& s : x1) s = 0.0; for (auto& s : F1x1) s = 0.0; }

    void setMode (Mode m) { mode = m; }
    void setParameters (float drivePercent, float ceilingLin, float mixPercent)
    {
        drive   = 1.0f + (drivePercent / 100.0f) * 11.0f;   // 1..12x
        ceiling = ceilingLin < 1e-3f ? 1e-3f : ceilingLin;
        mix     = mixPercent / 100.0f;
    }

    void process (float* const* data, int numCh, int numSamples)
    {
        for (int ch = 0; ch < numCh; ++ch)
        {
            const size_t c = (size_t) (ch < (int) kMaxCh ? ch : kMaxCh - 1);
            auto* d = data[ch];
            for (int n = 0; n < numSamples; ++n)
            {
                const double x = (double) d[n] * drive / ceiling;
                double y;
                if (mode == Mode::Soft)
                    y = std::tanh (x);
                else
                {
                    // ADAA1 hard clip.
                    const double F1 = antiderivHard (x);
                    const double diff = x - x1[c];
                    y = (std::abs (diff) < 1e-6) ? hard (0.5 * (x + x1[c]))
                                                 : (F1 - F1x1[c]) / diff;
                    x1[c] = x; F1x1[c] = F1;
                }
                const float wet = (float) (y * ceiling);
                d[n] = d[n] * (1.0f - mix) + wet * mix;
            }
        }
    }

private:
    static double hard (double x) { return x < -1.0 ? -1.0 : (x > 1.0 ? 1.0 : x); }
    // Antiderivative of hard-clip: x^2/2 for |x|<=1, else |x| - 1/2.
    static double antiderivHard (double x)
    {
        return std::abs (x) <= 1.0 ? 0.5 * x * x : std::abs (x) - 0.5;
    }

    static constexpr size_t kMaxCh = 2;
    Mode mode = Mode::Hard;
    double x1[kMaxCh] { 0.0, 0.0 }, F1x1[kMaxCh] { 0.0, 0.0 };
    float drive = 1.0f, ceiling = 1.0f, mix = 1.0f;
};
} // namespace aur
