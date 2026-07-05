#pragma once

#include <juce_dsp/juce_dsp.h>

namespace aur
{
/** Click-free smoothed gain stage, gain set in decibels. Realtime-safe. */
class GainStage
{
public:
    void prepare (double sampleRate) { sm.reset (sampleRate, 0.02); }
    void reset()                     { sm.setCurrentAndTargetValue (sm.getTargetValue()); }
    void setGainDecibels (float dB)  { sm.setTargetValue (juce::Decibels::decibelsToGain (dB)); }

    void process (juce::dsp::AudioBlock<float>& block)
    {
        const auto ns = block.getNumSamples();
        const auto nc = block.getNumChannels();
        for (size_t s = 0; s < ns; ++s)
        {
            const auto g = sm.getNextValue();
            for (size_t ch = 0; ch < nc; ++ch)
                block.getChannelPointer (ch)[s] *= g;
        }
    }

private:
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Multiplicative> sm { 1.0f };
};
} // namespace aur
