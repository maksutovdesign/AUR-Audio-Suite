#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

namespace ParamID
{
    static constexpr auto freq      = "freq";
    static constexpr auto threshold = "threshold";
    static constexpr auto range     = "range";
    static constexpr auto listen    = "listen";
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
            NormalisableRange<float> (3000.f, 12000.f, 1.f, 0.6f), 6500.f,
            AudioParameterFloatAttributes().withStringFromValueFunction ([](float v,int){ return v >= 1000.f ? String(v/1000.f,1)+" kHz" : String((int)v)+" Hz"; })));

        p.push_back (std::make_unique<AudioParameterFloat> (
            ParameterID { ParamID::threshold, 1 }, "Threshold",
            NormalisableRange<float> (-60.f, 0.f, 0.1f), -30.f,
            AudioParameterFloatAttributes().withStringFromValueFunction ([](float v,int){return String(v,1)+" dB";})));

        p.push_back (std::make_unique<AudioParameterFloat> (
            ParameterID { ParamID::range, 1 }, "Range",
            NormalisableRange<float> (0.f, 18.f, 0.1f), 10.f,
            AudioParameterFloatAttributes().withStringFromValueFunction ([](float v,int){return String(v,1)+" dB";})));

        p.push_back (std::make_unique<AudioParameterBool> (
            ParameterID { ParamID::listen, 1 }, "Listen", false));

        p.push_back (std::make_unique<AudioParameterBool> (
            ParameterID { ParamID::bypass, 1 }, "Bypass", false));

        return { p.begin(), p.end() };
    }
}
