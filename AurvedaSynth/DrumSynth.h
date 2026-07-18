#pragma once

#include <cmath>
#include <algorithm>
#include "Biquad.h"

namespace aur { namespace syn
{
inline float decayCoef (float seconds, double sr)
{
    return (float) std::exp (-6.90775527898 / (std::max (1.0e-4, (double) seconds) * sr));
}
inline float frand (unsigned& s) { s = s * 1664525u + 1013904223u; return (float) (s >> 8) / 8388608.0f - 1.0f; }
constexpr double kTwoPi = 6.283185307179586;

/** Kick — sine with a fast downward pitch envelope + amp decay. */
struct KickVoice
{
    void prepare (double s) { sr = s; hp.setHighpass (sr, 25.0, 0.7); }
    void setParams (float tune, float decay)
    {
        baseF  = 32.0f + tune * 90.0f;
        ampC   = decayCoef (0.12f + decay * 0.9f, sr);
        pC     = decayCoef (0.018f + decay * 0.02f, sr);
    }
    void trigger (float vel) { amp = vel; pEnv = 1.0f; phase = 0.0; click = vel * 0.7f; }
    float next()
    {
        const double f = baseF * (1.0 + 4.5 * pEnv);
        pEnv *= pC;
        phase += f / sr; if (phase >= 1.0) phase -= 1.0;
        float o = (float) std::sin (kTwoPi * phase) * amp;
        o += click; click *= 0.35f;                    // transient click
        amp *= ampC;
        return hp.process (o);
    }
    double sr = 44100, phase = 0; float baseF = 55, amp = 0, ampC = 0, pEnv = 0, pC = 0, click = 0;
    Biquad hp;
};

/** Snare — two body sines + highpassed noise, separate decays. */
struct SnareVoice
{
    void prepare (double s) { sr = s; hp.setHighpass (sr, 1200.0, 0.7); }
    void setParams (float tune, float decay)
    {
        f1 = 160.0f + tune * 180.0f; f2 = f1 * 1.58f;
        toneC  = decayCoef (0.06f + decay * 0.15f, sr);
        noiseC = decayCoef (0.08f + decay * 0.4f, sr);
    }
    void trigger (float vel) { tone = vel; noise = vel; p1 = p2 = 0.0; }
    float next()
    {
        p1 += f1 / sr; if (p1 >= 1) p1 -= 1;
        p2 += f2 / sr; if (p2 >= 1) p2 -= 1;
        const float body = (float) (std::sin (kTwoPi * p1) + 0.7 * std::sin (kTwoPi * p2)) * 0.5f * tone;
        const float nz = hp.process (frand (rng)) * noise;
        tone *= toneC; noise *= noiseC;
        return body * 0.6f + nz * 0.8f;
    }
    double sr = 44100, p1 = 0, p2 = 0; float f1 = 180, f2 = 300, tone = 0, toneC = 0, noise = 0, noiseC = 0;
    unsigned rng = 0x9e3779b9; Biquad hp;
};

/** Clap — several noise bursts through a bandpass, then a tail. */
struct ClapVoice
{
    void prepare (double s) { sr = s; bp.setBandpass (sr, 1100.0, 1.2); }
    void setParams (float tune, float decay)
    {
        bp.setBandpass (sr, 900.0 + tune * 900.0, 1.2);
        tailC = decayCoef (0.12f + decay * 0.4f, sr);
        burstSpacing = (int) (0.010 * sr);
    }
    void trigger (float vel) { level = vel; bursts = 3; timer = 0; env = vel; tail = 0; }
    float next()
    {
        float e = 0.0f;
        if (bursts > 0)
        {
            e = env;
            if (--timer <= 0) { --bursts; timer = burstSpacing; env = level; if (bursts == 0) tail = level; }
            else env *= 0.6f;   // fast intra-burst decay
        }
        else { e = tail; tail *= tailC; }
        return bp.process (frand (rng)) * e;
    }
    double sr = 44100; float level = 0, env = 0, tail = 0, tailC = 0; int bursts = 0, timer = 0, burstSpacing = 441;
    unsigned rng = 0x1234567; Biquad bp;
};

/** 808-style metallic hat — 6 inharmonic squares → highpass → decay. */
struct HatVoice
{
    void prepare (double s) { sr = s; hp.setHighpass (sr, 7000.0, 0.7); }
    void setParams (float tune, float decay, bool open)
    {
        fund = 300.0f + tune * 200.0f;
        ampC = decayCoef ((open ? 0.35f : 0.045f) + decay * (open ? 0.6f : 0.12f), sr);
        hp.setHighpass (sr, 6000.0 + tune * 3000.0, 0.7);
    }
    void trigger (float vel) { amp = vel; for (auto& p : ph) p = 0.0; }
    float next()
    {
        static const double ratio[6] = { 2.0, 3.03, 4.16, 5.43, 6.79, 8.21 };
        float sum = 0.0f;
        for (int i = 0; i < 6; ++i)
        {
            ph[i] += fund * ratio[i] / sr; if (ph[i] >= 1) ph[i] -= 1;
            sum += ph[i] < 0.5 ? 1.0f : -1.0f;         // square
        }
        amp *= ampC;
        return hp.process (sum * (1.0f / 6.0f)) * amp;
    }
    void choke() { amp *= 0.25f; }
    double sr = 44100, ph[6] { }; float fund = 400, amp = 0, ampC = 0;
    Biquad hp;
};

/** Tom — sine with a gentle downward pitch envelope. */
struct TomVoice
{
    void prepare (double s) { sr = s; }
    void setParams (float tune, float decay) { baseF = 90.0f + tune * 220.0f; ampC = decayCoef (0.15f + decay * 0.6f, sr); pC = decayCoef (0.08f, sr); }
    void trigger (float vel) { amp = vel; pEnv = 1.0f; phase = 0.0; }
    float next()
    {
        const double f = baseF * (1.0 + 1.2 * pEnv); pEnv *= pC;
        phase += f / sr; if (phase >= 1) phase -= 1;
        const float o = (float) std::sin (kTwoPi * phase) * amp; amp *= ampC;
        return o;
    }
    double sr = 44100, phase = 0; float baseF = 140, amp = 0, ampC = 0, pEnv = 0, pC = 0;
};

/** Rim — short resonant band-passed burst. */
struct RimVoice
{
    void prepare (double s) { sr = s; bp.setBandpass (sr, 1700.0, 3.0); }
    void setParams (float tune, float decay) { bp.setBandpass (sr, 1400.0 + tune * 1200.0, 3.0); ampC = decayCoef (0.02f + decay * 0.08f, sr); }
    void trigger (float vel) { amp = vel; }
    float next() { const float o = bp.process (frand (rng)) * amp; amp *= ampC; return o; }
    double sr = 44100; float amp = 0, ampC = 0; unsigned rng = 0x55aa55aa; Biquad bp;
};
}} // namespace aur::syn
