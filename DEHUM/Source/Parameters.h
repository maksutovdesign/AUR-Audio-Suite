#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

namespace ParamID
{
    static constexpr auto freq      = "freq";
    static constexpr auto harmonics = "harmonics";
    static constexpr auto depth     = "depth";
    static constexpr auto q         = "q";
    static constexpr auto bypass    = "bypass";
}

namespace Params
{
    inline juce::AudioProcessorValueTreeState::ParameterLayout createLayout()
    {
        using namespace juce;
        std::vector<std::unique_ptr<RangedAudioParameter>> p;

        p.push_back (std::make_unique<AudioParameterFloat> (
            ParameterID { ParamID::freq, 1 }, "Freq",
            NormalisableRange<float> (45.f, 65.f, 0.1f), 50.f,
            AudioParameterFloatAttributes().withStringFromValueFunction ([](float v,int){return String(v,1)+" Hz";})));

        p.push_back (std::make_unique<AudioParameterInt> (
            ParameterID { ParamID::harmonics, 1 }, "Harmonics", 1, 8, 4));

        p.push_back (std::make_unique<AudioParameterFloat> (
            ParameterID { ParamID::depth, 1 }, "Depth",
            NormalisableRange<float> (3.f, 48.f, 0.1f), 24.f,
            AudioParameterFloatAttributes().withStringFromValueFunction ([](float v,int){return String(v,1)+" dB";})));

        p.push_back (std::make_unique<AudioParameterFloat> (
            ParameterID { ParamID::q, 1 }, "Width",
            NormalisableRange<float> (5.f, 40.f, 0.1f, 0.6f), 20.f,
            AudioParameterFloatAttributes().withStringFromValueFunction ([](float v,int){return "Q " + String((int)v);})));

        p.push_back (std::make_unique<AudioParameterBool> (
            ParameterID { ParamID::bypass, 1 }, "Bypass", false));

        return { p.begin(), p.end() };
    }
}
