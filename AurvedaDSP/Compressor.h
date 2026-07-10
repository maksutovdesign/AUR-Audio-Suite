#pragma once

#include <juce_dsp/juce_dsp.h>

namespace aur
{
/**
    Feed-forward peak compressor with soft knee, attack/release envelope,
    makeup gain and parallel (dry/wet) mix. Stereo-linked detector so the
    image is preserved. Exposes current gain reduction (dB, >= 0) for metering.
    Realtime-safe: no allocation in process().
*/
class Compressor
{
public:
    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        sampleRate = spec.sampleRate;
        makeupSm.reset (sampleRate, 0.02);
        mixSm.reset    (sampleRate, 0.02);
        reset();
        updateCoefficients();
    }

    void reset()
    {
        envelope = 0.0f;
        currentGrDb = 0.0f;
        makeupSm.setCurrentAndTargetValue (makeupSm.getTargetValue());
        mixSm.setCurrentAndTargetValue    (mixSm.getTargetValue());
    }

    void setParameters (float thresholdDb, float ratioIn, float attackMs,
                        float releaseMs, float makeupDb, float mixPercent)
    {
        threshold = thresholdDb;
        ratio   = juce::jmax (1.0f, ratioIn);
        attack  = attackMs;
        release = releaseMs;
        makeupSm.setTargetValue (juce::Decibels::decibelsToGain (makeupDb));
        mixSm.setTargetValue    (mixPercent / 100.0f);
        updateCoefficients();
    }

    void process (juce::dsp::AudioBlock<float>& block)
    {
        const auto numSamples  = block.getNumSamples();
        const auto numChannels = block.getNumChannels();
        float maxGr = 0.0f;

        for (size_t s = 0; s < numSamples; ++s)
        {
            float key = 0.0f;
            for (size_t ch = 0; ch < numChannels; ++ch)
                key = juce::jmax (key, std::abs (block.getChannelPointer (ch)[s]));

            const auto coeff = key > envelope ? attackCoeff : releaseCoeff;
            envelope = coeff * (envelope - key) + key;

            const auto envDb = juce::Decibels::gainToDecibels (envelope, -100.0f);
            const auto grDb  = computeGainReduction (envDb);
            const auto gain  = juce::Decibels::decibelsToGain (-grDb);
            maxGr = juce::jmax (maxGr, grDb);

            const auto makeup = makeupSm.getNextValue();
            const auto mix    = mixSm.getNextValue();

            for (size_t ch = 0; ch < numChannels; ++ch)
            {
                auto* d = block.getChannelPointer (ch);
                const auto dry = d[s];
                const auto wet = dry * gain * makeup;
                d[s] = dry * (1.0f - mix) + wet * mix;
            }
        }
        currentGrDb = maxGr;
    }

    float getGainReductionDb() const noexcept { return currentGrDb; }

private:
    void updateCoefficients()
    {
        attackCoeff  = std::exp (-1.0f / (float) (0.001 * attack  * sampleRate));
        releaseCoeff = std::exp (-1.0f / (float) (0.001 * release * sampleRate));
    }

    float computeGainReduction (float inputDb) const
    {
        const float knee = 6.0f;
        const float over = inputDb - threshold;
        if (over <= -knee * 0.5f) return 0.0f;

        float outOver;
        if (over >= knee * 0.5f)
            outOver = over / ratio;
        else
        {
            const float x = over + knee * 0.5f;
            outOver = over - (1.0f - 1.0f / ratio) * (x * x) / (2.0f * knee);
        }
        return juce::jmax (0.0f, over - outOver);
    }

    double sampleRate = 44100.0;
    float threshold = 0.0f, ratio = 2.0f, attack = 10.0f, release = 100.0f;
    float attackCoeff = 0.0f, releaseCoeff = 0.0f;
    float envelope = 0.0f, currentGrDb = 0.0f;

    juce::SmoothedValue<float> makeupSm { 1.0f };
    juce::SmoothedValue<float> mixSm    { 1.0f };
};
} // namespace aur
