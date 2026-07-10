#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

namespace ParamID
{
    static constexpr auto threshold = "threshold";
    static constexpr auto ratio     = "ratio";
    static constexpr auto attack    = "attack";
    static constexpr auto release   = "release";
    static constexpr auto makeup    = "makeup";
    static constexpr auto mix       = "mix";
    static constexpr auto bypass    = "bypass";
}

namespace Params
{
    inline juce::AudioProcessorValueTreeState::ParameterLayout createLayout()
    {
        using namespace juce;
        std::vector<std::unique_ptr<RangedAudioParameter>> p;
        auto db = [](float v){ return String (v, 1) + " dB"; };

        p.push_back (std::make_unique<AudioParameterFloat> (
            ParameterID { ParamID::threshold, 1 }, "Threshold",
            NormalisableRange<float> (-60.f, 0.f, 0.1f), -18.f,
            AudioParameterFloatAttributes().withStringFromValueFunction ([db](float v,int){return db(v);})));

        p.push_back (std::make_unique<AudioParameterFloat> (
            ParameterID { ParamID::ratio, 1 }, "Ratio",
            NormalisableRange<float> (1.f, 20.f, 0.1f, 0.5f), 3.f,
            AudioParameterFloatAttributes().withStringFromValueFunction ([](float v,int){return String(v,1)+":1";})));

        p.push_back (std::make_unique<AudioParameterFloat> (
            ParameterID { ParamID::attack, 1 }, "Attack",
            NormalisableRange<float> (0.1f, 200.f, 0.1f, 0.4f), 10.f,
            AudioParameterFloatAttributes().withLabel ("ms")));

        p.push_back (std::make_unique<AudioParameterFloat> (
            ParameterID { ParamID::release, 1 }, "Release",
            NormalisableRange<float> (5.f, 1000.f, 1.f, 0.4f), 120.f,
            AudioParameterFloatAttributes().withLabel ("ms")));

        p.push_back (std::make_unique<AudioParameterFloat> (
            ParameterID { ParamID::makeup, 1 }, "Makeup",
            NormalisableRange<float> (0.f, 24.f, 0.1f), 0.f,
            AudioParameterFloatAttributes().withStringFromValueFunction ([db](float v,int){return db(v);})));

        p.push_back (std::make_unique<AudioParameterFloat> (
            ParameterID { ParamID::mix, 1 }, "Mix",
            NormalisableRange<float> (0.f, 100.f, 0.1f), 100.f,
            AudioParameterFloatAttributes().withLabel ("%")));

        p.push_back (std::make_unique<AudioParameterBool> (
            ParameterID { ParamID::bypass, 1 }, "Bypass", false));

        return { p.begin(), p.end() };
    }
}
