#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

namespace ParamID
{
    static constexpr auto size     = "size";
    static constexpr auto decay    = "decay";
    static constexpr auto damp      = "damp";
    static constexpr auto predelay = "predelay";
    static constexpr auto width    = "width";
    static constexpr auto mix      = "mix";
    static constexpr auto bypass   = "bypass";
}

namespace Params
{
    inline juce::AudioProcessorValueTreeState::ParameterLayout createLayout()
    {
        using namespace juce;
        std::vector<std::unique_ptr<RangedAudioParameter>> p;
        auto pct = [](float v,int){ return String ((int) v) + "%"; };

        p.push_back (std::make_unique<AudioParameterFloat> (
            ParameterID { ParamID::size, 1 }, "Size",
            NormalisableRange<float> (0.f, 100.f, 0.1f), 55.f,
            AudioParameterFloatAttributes().withStringFromValueFunction (pct)));

        p.push_back (std::make_unique<AudioParameterFloat> (
            ParameterID { ParamID::decay, 1 }, "Decay",
            NormalisableRange<float> (0.2f, 10.f, 0.01f, 0.4f), 2.5f,
            AudioParameterFloatAttributes().withStringFromValueFunction ([](float v,int){return String(v,2)+" s";})));

        p.push_back (std::make_unique<AudioParameterFloat> (
            ParameterID { ParamID::damp, 1 }, "Damp",
            NormalisableRange<float> (0.f, 100.f, 0.1f), 40.f,
            AudioParameterFloatAttributes().withStringFromValueFunction (pct)));

        p.push_back (std::make_unique<AudioParameterFloat> (
            ParameterID { ParamID::predelay, 1 }, "Pre-Delay",
            NormalisableRange<float> (0.f, 150.f, 1.f), 20.f,
            AudioParameterFloatAttributes().withLabel ("ms")));

        p.push_back (std::make_unique<AudioParameterFloat> (
            ParameterID { ParamID::width, 1 }, "Width",
            NormalisableRange<float> (0.f, 200.f, 1.f), 100.f,
            AudioParameterFloatAttributes().withStringFromValueFunction (pct)));

        p.push_back (std::make_unique<AudioParameterFloat> (
            ParameterID { ParamID::mix, 1 }, "Mix",
            NormalisableRange<float> (0.f, 100.f, 0.1f), 30.f,
            AudioParameterFloatAttributes().withStringFromValueFunction (pct)));

        p.push_back (std::make_unique<AudioParameterBool> (
            ParameterID { ParamID::bypass, 1 }, "Bypass", false));

        return { p.begin(), p.end() };
    }
}
