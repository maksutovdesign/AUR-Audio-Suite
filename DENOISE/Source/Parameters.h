#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

namespace ParamID
{
    static constexpr auto amount      = "amount";
    static constexpr auto sensitivity = "sensitivity";
    static constexpr auto bypass      = "bypass";
}

namespace Params
{
    inline juce::AudioProcessorValueTreeState::ParameterLayout createLayout()
    {
        using namespace juce;
        std::vector<std::unique_ptr<RangedAudioParameter>> p;
        auto pct = [](float v,int){ return String((int)v)+"%"; };

        p.push_back (std::make_unique<AudioParameterFloat> (
            ParameterID { ParamID::amount, 1 }, "Amount",
            NormalisableRange<float> (0.f, 100.f, 0.1f), 70.f,
            AudioParameterFloatAttributes().withStringFromValueFunction (pct)));

        p.push_back (std::make_unique<AudioParameterFloat> (
            ParameterID { ParamID::sensitivity, 1 }, "Sensitivity",
            NormalisableRange<float> (0.f, 100.f, 0.1f), 40.f,
            AudioParameterFloatAttributes().withStringFromValueFunction (pct)));

        p.push_back (std::make_unique<AudioParameterBool> (
            ParameterID { ParamID::bypass, 1 }, "Bypass", false));

        return { p.begin(), p.end() };
    }
}
