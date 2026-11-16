#pragma once

#include "LoudnessMeter.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace aur
{
/**
    Brings a processed signal to a loudness target by listening to it (used by ASSIST).

    An assistant that sets its gain from the loudness of its INPUT misses the target by however
    much the stages after the gain (a tilt, a compressor, a limiter) take off: in ASSIST that is
    3 to 4.5 dB. After the analysis, call start(); then feed the OUTPUT through push(). It
    listens in up to three rounds of 1.5 s (skipping 0.25 s while the last change settles) and
    corrects the gain each round until the output is within 0.5 dB of the target, or of the part
    of the way `intensity` asks for. From the second round it steps by how far the output moved
    for the gain it added (above a compressor's threshold, about 0.4 dB per dB).

    Realtime-safe after prepare(): no allocation in start() or push().
*/
class LoudnessTrim
{
public:
    static constexpr int   kRounds    = 3;
    static constexpr float kTolerance = 0.5f;  // dB
    static constexpr float kMaxGain   = 30.0f; // dB either way

    void prepare (double sampleRate, int numChannels)
    {
        fs = sampleRate;
        meter.prepare (sampleRate, numChannels);
        rounds = 0;
    }

    /** The analysis is done: `inputLufs` is what it measured on the input. */
    void start (float inputLufs) noexcept
    {
        inLufs = inputLufs;
        rounds = kRounds;
        slope = 1.0f;
        lastOut = std::numeric_limits<float>::quiet_NaN();
        lastApplied = 0.0f;
        beginRound();
    }

    void stop() noexcept { rounds = 0; }
    bool isActive() const noexcept { return rounds > 0; }

    /** Feed n frames of the output. `gainDb` is the assistant's gain before intensity scales it;
        returns true when it was changed. */
    bool push (const float* const* out, size_t numCh, int n, float targetLufs, float intensity, float& gainDb) noexcept
    {
        if (rounds <= 0)
            return false;
        for (int i = 0; i < n; ++i)
        {
            if (skip > 0) { --skip; continue; }
            const float* frame[2] = { out[0] + i, out[numCh > 1 ? 1 : 0] + i };
            meter.push (frame, numCh > 1 ? 2 : 1);
            --remaining;
        }
        if (remaining > 0)
            return false;

        const float measured = meter.shortTermLufs();
        const float wanted = inLufs + intensity * (targetLufs - inLufs);
        const float error = wanted - measured;
        --rounds;
        if (intensity < 0.01f || std::abs (error) < kTolerance || ! std::isfinite (measured))
        {
            rounds = 0;
            return false;
        }
        const float applied = gainDb * intensity;
        if (std::isfinite (lastOut) && std::abs (applied - lastApplied) > 0.1f)
            slope = std::clamp ((measured - lastOut) / (applied - lastApplied), 0.25f, 1.5f);
        lastOut = measured;
        lastApplied = applied;
        gainDb = std::clamp (gainDb + error / slope / intensity, -kMaxGain, kMaxGain);
        if (rounds > 0)
            beginRound();
        return true;
    }

private:
    void beginRound() noexcept
    {
        skip = (int) (0.25 * fs);
        remaining = (int) (1.25 * fs);
        meter.reset();
    }

    LoudnessMeter meter;
    double fs = 48000.0;
    int rounds = 0, skip = 0, remaining = 0;
    float inLufs = -70.0f, slope = 1.0f, lastOut = 0.0f, lastApplied = 0.0f;
};
} // namespace aur
