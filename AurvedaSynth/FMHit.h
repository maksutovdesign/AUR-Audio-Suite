#pragma once
#include "DrumSynth.h"

namespace aur { namespace syn
{
/** Generic 2-op FM percussion hit: carrier + modulator, both with exponential
    decays. configure() gives each pad its character; tune/decay stay live. */
struct FMHitVoice
{
    void prepare (double s) { sr = s; hp.setHighpass (sr, 25.0, 0.7); }

    /** carHz0: base carrier at tune=0.5; ratio: mod/car; index0: FM depth;
        decMul: pad decay scale; modDecRel: mod env speed vs amp env. */
    void configure (float carHz0, float ratio_, float index0_, float decMul_, float modDecRel_)
    { carHz = carHz0; ratio = ratio_; index0 = index0_; decMul = decMul_; modDecRel = modDecRel_; }

    void setParams (float tune, float decay)
    {
        f0 = carHz * std::pow (2.0f, (tune - 0.5f) * 2.0f);
        const float d = (0.05f + decay * 0.6f) * decMul;
        ampC = decayCoef (d, sr);
        modC = decayCoef (d * modDecRel, sr);
    }
    void setParams (float tune, float decay, bool open) { setParams (tune, decay * (open ? 1.6f : 0.6f)); }

    void trigger (float vel) { amp = vel; mod = 1.0f; pc = pm = 0.0; }
    void choke () { amp *= 0.2f; }

    float next()
    {
        pm += f0 * ratio / sr; if (pm >= 1) pm -= 1;
        const float m = (float) std::sin (kTwoPi * pm) * index0 * mod;
        pc += f0 / sr; if (pc >= 1) pc -= 1;
        const float o = (float) std::sin (kTwoPi * pc + m) * amp;
        amp *= ampC; mod *= modC;
        return hp.process (o);
    }

    double sr = 44100, pc = 0, pm = 0;
    float carHz = 200, ratio = 1.4f, index0 = 4.f, decMul = 1.f, modDecRel = 0.4f;
    float f0 = 200, amp = 0, ampC = 0, mod = 0, modC = 0;
    Biquad hp;
};
}} // namespace aur::syn
