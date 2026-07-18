#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

namespace ParamID
{
    static constexpr auto character = "character"; // synthetic IR type
    static constexpr auto decay    = "decay";     // reverb tail length (s)
    static constexpr auto tone     = "tone";      // IR brightness / damping (%)
    static constexpr auto predelay = "predelay";  // ms of silence before the tail
    static constexpr auto width    = "width";     // stereo decorrelation of the tail (%)
    static constexpr auto mix      = "mix";       // dry/wet (%)
    static constexpr auto bypass   = "bypass";
}

namespace Params
{
    inline juce::AudioProcessorValueTreeState::ParameterLayout createLayout()
    {
        using namespace juce;
        std::vector<std::unique_ptr<RangedAudioParameter>> p;
        auto pct = [](float v,int){ return String ((int) v) + " %"; };

        p.push_back (std::make_unique<AudioParameterChoice> (
            ParameterID { ParamID::character, 1 }, "Character",
            StringArray { "Smooth", "Room", "Plate", "Hall", "Spring" }, 0));

        p.push_back (std::make_unique<AudioParameterFloat> (
            ParameterID { ParamID::decay, 1 }, "Decay",
            NormalisableRange<float> (0.2f, 6.0f, 0.01f, 0.5f), 1.8f,
            AudioParameterFloatAttributes().withStringFromValueFunction ([](float v,int){return String(v,2)+" s";})));

        p.push_back (std::make_unique<AudioParameterFloat> (
            ParameterID { ParamID::tone, 1 }, "Tone",
            NormalisableRange<float> (0.f, 100.f, 0.1f), 55.f,
            AudioParameterFloatAttributes().withStringFromValueFunction (pct)));

        p.push_back (std::make_unique<AudioParameterFloat> (
            ParameterID { ParamID::predelay, 1 }, "Pre-Delay",
            NormalisableRange<float> (0.f, 150.f, 0.1f), 12.f,
            AudioParameterFloatAttributes().withLabel ("ms")));

        p.push_back (std::make_unique<AudioParameterFloat> (
            ParameterID { ParamID::width, 1 }, "Width",
            NormalisableRange<float> (0.f, 100.f, 0.1f), 100.f,
            AudioParameterFloatAttributes().withStringFromValueFunction (pct)));

        p.push_back (std::make_unique<AudioParameterFloat> (
            ParameterID { ParamID::mix, 1 }, "Mix",
            NormalisableRange<float> (0.f, 100.f, 0.1f), 35.f,
            AudioParameterFloatAttributes().withStringFromValueFunction (pct)));

        p.push_back (std::make_unique<AudioParameterBool> (
            ParameterID { ParamID::bypass, 1 }, "Bypass", false));

        return { p.begin(), p.end() };
    }
}
