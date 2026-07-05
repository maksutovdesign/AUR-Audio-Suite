#pragma once

#include <juce_dsp/juce_dsp.h>

namespace aur
{
/** Single-knob tilt tone: mirrored low-shelf / high-shelf around ~700 Hz.
    tone < 0 → darker, tone > 0 → brighter. */
class ToneTilt
{
public:
    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        fs = spec.sampleRate;
        low.prepare (spec);
        high.prepare (spec);
        setTone (0.0f);
    }

    void reset() { low.reset(); high.reset(); }

    /** tone: -100..100 */
    void setTone (float tone)
    {
        const float g = (tone / 100.0f) * 9.0f; // ±9 dB
        *low.state  = *juce::dsp::IIR::Coefficients<float>::makeLowShelf  (fs, 250.0f,  0.707f, juce::Decibels::decibelsToGain (-g));
        *high.state = *juce::dsp::IIR::Coefficients<float>::makeHighShelf (fs, 4000.0f, 0.707f, juce::Decibels::decibelsToGain ( g));
    }

    void process (juce::dsp::AudioBlock<float>& block)
    {
        juce::dsp::ProcessContextReplacing<float> ctx (block);
        low.process (ctx);
        high.process (ctx);
    }

private:
    using Filter = juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>,
                                                  juce::dsp::IIR::Coefficients<float>>;
    Filter low, high;
    double fs = 44100.0;
};
} // namespace aur
