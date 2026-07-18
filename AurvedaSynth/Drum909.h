#pragma once
#include "DrumSynth.h"

namespace aur { namespace syn
{
/** 909-style kick: faster/harder sine sweep + saturating click. */
struct Kick9Voice
{
    void prepare (double s) { sr = s; hp.setHighpass (sr, 30.0, 0.7); }
    void setParams (float tune, float decay)
    {
        baseF = 45.0f + tune * 60.0f;
        ampC  = decayCoef (0.08f + decay * 0.5f, sr);
        pC    = decayCoef (0.008f + decay * 0.008f, sr);
    }
    void trigger (float vel) { amp = vel; pEnv = 1.0f; phase = 0.0; click = vel; }
    float next()
    {
        const double f = baseF * (1.0 + 8.0 * pEnv);
        pEnv *= pC;
        phase += f / sr; if (phase >= 1.0) phase -= 1.0;
        float o = std::tanh ((float) std::sin (kTwoPi * phase) * 1.6f) * amp;
        o += click; click *= 0.5f;
        amp *= ampC;
        return hp.process (o);
    }
    double sr = 44100, phase = 0; float baseF = 60, amp = 0, ampC = 0, pEnv = 0, pC = 0, click = 0;
    Biquad hp;
};

/** 909 snare: two detuned sines + bright noise snap. */
struct Snare9Voice
{
    void prepare (double s) { sr = s; hp.setHighpass (sr, 2000.0, 0.7); }
    void setParams (float tune, float decay)
    {
        f1 = 185.0f + tune * 150.0f; f2 = f1 * 1.44f;
        toneC  = decayCoef (0.05f + decay * 0.1f, sr);
        noiseC = decayCoef (0.05f + decay * 0.25f, sr);
    }
    void trigger (float vel) { tone = vel; noise = vel; p1 = p2 = 0.0; }
    float next()
    {
        p1 += f1 / sr; if (p1 >= 1) p1 -= 1;
        p2 += f2 / sr; if (p2 >= 1) p2 -= 1;
        const float body = (float) (std::sin (kTwoPi * p1) * 0.6 + std::sin (kTwoPi * p2) * 0.4) * tone;
        const float nz = hp.process (frand (rng)) * noise;
        tone *= toneC; noise *= noiseC;
        return std::tanh (body * 0.7f + nz * 1.1f);
    }
    double sr = 44100, p1 = 0, p2 = 0; float f1 = 200, f2 = 290, tone = 0, toneC = 0, noise = 0, noiseC = 0;
    unsigned rng = 0xbeef1234; Biquad hp;
};

/** 909 hat: HP noise + square metal blend (brighter than 808). */
struct Hat9Voice
{
    void prepare (double s) { sr = s; hp.setHighpass (sr, 8000.0, 0.7); }
    void setParams (float tune, float decay, bool open)
    {
        fund = 320.0f + tune * 260.0f;
        ampC = decayCoef ((open ? 0.3f : 0.035f) + decay * (open ? 0.5f : 0.1f), sr);
        hp.setHighpass (sr, 7000.0 + tune * 4000.0, 0.7);
    }
    void trigger (float vel) { amp = vel; for (auto& p : ph) p = 0.0; }
    float next()
    {
        static const double ratio[4] = { 2.0, 3.31, 4.73, 6.17 };
        float sum = 0.0f;
        for (int i = 0; i < 4; ++i)
        {
            ph[i] += fund * ratio[i] / sr; if (ph[i] >= 1) ph[i] -= 1;
            sum += ph[i] < 0.5 ? 1.0f : -1.0f;
        }
        rngS = rngS * 1664525u + 1013904223u;
        const float nz = (float) (rngS >> 8) / 8388608.0f - 1.0f;
        amp *= ampC;
        return hp.process (sum * 0.12f + nz * 0.6f) * amp;
    }
    void choke() { amp *= 0.2f; }
    double sr = 44100, ph[4] { }; float fund = 400, amp = 0, ampC = 0;
    unsigned rngS = 0x909909; Biquad hp;
};
}} // namespace aur::syn
