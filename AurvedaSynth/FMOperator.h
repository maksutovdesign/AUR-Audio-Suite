#pragma once

#include <cmath>

namespace aur { namespace syn
{
/** One FM (phase-modulation) operator: a sine whose phase is offset each sample
    by an external modulation input (in radians). Envelope/level live in the
    voice, not here. */
struct FMOperator
{
    void prepare (double sampleRate) { sr = sampleRate; }
    void reset (double startPhase = 0.0) { phase = startPhase; last = 0.0f; }
    void setFrequency (double hz) { inc = hz / sr; }

    /** Render with an external phase-modulation input (radians). */
    inline float render (float phaseModRadians)
    {
        const float out = (float) std::sin (6.283185307179586 * phase + phaseModRadians);
        phase += inc; if (phase >= 1.0) phase -= 1.0;
        last = out;
        return out;
    }

    float lastOutput() const { return last; }

    double sr = 44100.0, phase = 0.0, inc = 0.0;
    float  last = 0.0f;
};
}} // namespace aur::syn
