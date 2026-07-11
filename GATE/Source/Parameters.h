#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

namespace ParamID
{
    static constexpr auto threshold = "threshold";
    static constexpr auto range     = "range";
    static constexpr auto attack    = "attack";
    static constexpr auto hold      = "hold";
    static constexpr auto release   = "release";
    static constexpr auto bypass    = "bypass";
}

namespace Params
{
    inline juce::AudioProcessorValueTreeState::ParameterLayout createLayout()
    {
        using namespace juce;
        std::vector<std::unique_ptr<RangedAudioParameter>> p;
        auto ms = [](float v,int){ return String(v,1)+" ms"; };

        p.push_back (std::make_unique<AudioParameterFloat> (
            ParameterID { ParamID::threshold, 1 }, "Threshold",
            NormalisableRange<float> (-80.f, 0.f, 0.1f), -40.f,
            AudioParameterFloatAttributes().withStringFromValueFunction ([](float v,int){return String(v,1)+" dB";})));

        p.push_back (std::make_unique<AudioParameterFloat> (
            ParameterID { ParamID::range, 1 }, "Range",
            NormalisableRange<float> (6.f, 90.f, 0.1f), 60.f,
            AudioParameterFloatAttributes().withStringFromValueFunction ([](float v,int){return String(v,1)+" dB";})));

        p.push_back (std::make_unique<AudioParameterFloat> (
            ParameterID { ParamID::attack, 1 }, "Attack",
            NormalisableRange<float> (0.1f, 100.f, 0.1f, 0.4f), 1.f,
            AudioParameterFloatAttributes().withStringFromValueFunction (ms)));

        p.push_back (std::make_unique<AudioParameterFloat> (
            ParameterID { ParamID::hold, 1 }, "Hold",
            NormalisableRange<float> (0.f, 500.f, 1.f, 0.5f), 50.f,
            AudioParameterFloatAttributes().withStringFromValueFunction (ms)));

        p.push_back (std::make_unique<AudioParameterFloat> (
            ParameterID { ParamID::release, 1 }, "Release",
            NormalisableRange<float> (5.f, 2000.f, 1.f, 0.4f), 100.f,
            AudioParameterFloatAttributes().withStringFromValueFunction (ms)));

        p.push_back (std::make_unique<AudioParameterBool> (
            ParameterID { ParamID::bypass, 1 }, "Bypass", false));

        return { p.begin(), p.end() };
    }
}
