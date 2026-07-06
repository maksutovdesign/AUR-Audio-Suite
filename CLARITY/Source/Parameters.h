#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

namespace ParamID
{
    static constexpr auto depth     = "depth";
    static constexpr auto sens      = "sens";
    static constexpr auto sharpness = "sharpness";
    static constexpr auto mix       = "mix";
    static constexpr auto delta     = "delta";
    static constexpr auto bypass    = "bypass";
}

namespace Params
{
    inline juce::AudioProcessorValueTreeState::ParameterLayout createLayout()
    {
        using namespace juce;
        std::vector<std::unique_ptr<RangedAudioParameter>> p;

        auto pct = [](float v){ return String ((int) v) + "%"; };

        p.push_back (std::make_unique<AudioParameterFloat> (
            ParameterID { ParamID::depth, 1 }, "Depth",
            NormalisableRange<float> (0.f, 100.f, 0.1f), 55.f,
            AudioParameterFloatAttributes().withStringFromValueFunction ([pct](float v,int){return pct(v);})));

        p.push_back (std::make_unique<AudioParameterFloat> (
            ParameterID { ParamID::sens, 1 }, "Sensitivity",
            NormalisableRange<float> (0.f, 100.f, 0.1f), 50.f,
            AudioParameterFloatAttributes().withStringFromValueFunction ([pct](float v,int){return pct(v);})));

        p.push_back (std::make_unique<AudioParameterFloat> (
            ParameterID { ParamID::sharpness, 1 }, "Sharpness",
            NormalisableRange<float> (0.f, 100.f, 0.1f), 55.f,
            AudioParameterFloatAttributes().withStringFromValueFunction ([pct](float v,int){return pct(v);})));

        p.push_back (std::make_unique<AudioParameterFloat> (
            ParameterID { ParamID::mix, 1 }, "Mix",
            NormalisableRange<float> (0.f, 100.f, 0.1f), 100.f,
            AudioParameterFloatAttributes().withStringFromValueFunction ([pct](float v,int){return pct(v);})));

        p.push_back (std::make_unique<AudioParameterBool> (
            ParameterID { ParamID::delta, 1 }, "Delta Listen", false));

        p.push_back (std::make_unique<AudioParameterBool> (
            ParameterID { ParamID::bypass, 1 }, "Bypass", false));

        return { p.begin(), p.end() };
    }
}
