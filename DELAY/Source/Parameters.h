#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

namespace ParamID
{
    static constexpr auto time     = "time";
    static constexpr auto feedback = "feedback";
    static constexpr auto damp     = "damp";
    static constexpr auto width    = "width";
    static constexpr auto pingpong = "pingpong";
    static constexpr auto mix      = "mix";
    static constexpr auto bypass   = "bypass";
}

namespace Params
{
    inline juce::AudioProcessorValueTreeState::ParameterLayout createLayout()
    {
        using namespace juce;
        std::vector<std::unique_ptr<RangedAudioParameter>> p;
        auto pct = [](float v,int){ return String((int)v)+"%"; };

        p.push_back (std::make_unique<AudioParameterFloat> (
            ParameterID { ParamID::time, 1 }, "Time",
            NormalisableRange<float> (10.f, 2000.f, 1.f, 0.4f), 350.f,
            AudioParameterFloatAttributes().withStringFromValueFunction ([](float v,int){return String((int)v)+" ms";})));

        p.push_back (std::make_unique<AudioParameterFloat> (
            ParameterID { ParamID::feedback, 1 }, "Feedback",
            NormalisableRange<float> (0.f, 98.f, 0.1f), 35.f,
            AudioParameterFloatAttributes().withStringFromValueFunction (pct)));

        p.push_back (std::make_unique<AudioParameterFloat> (
            ParameterID { ParamID::damp, 1 }, "Damp",
            NormalisableRange<float> (0.f, 100.f, 0.1f), 40.f,
            AudioParameterFloatAttributes().withStringFromValueFunction (pct)));

        p.push_back (std::make_unique<AudioParameterFloat> (
            ParameterID { ParamID::width, 1 }, "Width",
            NormalisableRange<float> (0.f, 200.f, 1.f), 100.f,
            AudioParameterFloatAttributes().withStringFromValueFunction (pct)));

        p.push_back (std::make_unique<AudioParameterBool> (
            ParameterID { ParamID::pingpong, 1 }, "Ping-Pong", false));

        p.push_back (std::make_unique<AudioParameterFloat> (
            ParameterID { ParamID::mix, 1 }, "Mix",
            NormalisableRange<float> (0.f, 100.f, 0.1f), 30.f,
            AudioParameterFloatAttributes().withStringFromValueFunction (pct)));

        p.push_back (std::make_unique<AudioParameterBool> (
            ParameterID { ParamID::bypass, 1 }, "Bypass", false));

        return { p.begin(), p.end() };
    }
}
