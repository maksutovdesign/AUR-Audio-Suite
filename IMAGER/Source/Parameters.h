#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

namespace ParamID
{
    static constexpr auto width     = "width";
    static constexpr auto monoBelow = "monoBelow";
    static constexpr auto balance   = "balance";
    static constexpr auto bypass    = "bypass";
}

namespace Params
{
    inline juce::AudioProcessorValueTreeState::ParameterLayout createLayout()
    {
        using namespace juce;
        std::vector<std::unique_ptr<RangedAudioParameter>> p;

        p.push_back (std::make_unique<AudioParameterFloat> (
            ParameterID { ParamID::width, 1 }, "Width",
            NormalisableRange<float> (0.f, 200.f, 1.f), 100.f,
            AudioParameterFloatAttributes().withStringFromValueFunction ([](float v,int){return String((int)v)+"%";})));

        p.push_back (std::make_unique<AudioParameterFloat> (
            ParameterID { ParamID::monoBelow, 1 }, "Mono Below",
            NormalisableRange<float> (20.f, 500.f, 1.f, 0.4f), 20.f,
            AudioParameterFloatAttributes().withStringFromValueFunction ([](float v,int){ return v <= 21.f ? String("Off") : String((int)v)+" Hz"; })));

        p.push_back (std::make_unique<AudioParameterFloat> (
            ParameterID { ParamID::balance, 1 }, "Balance",
            NormalisableRange<float> (-1.f, 1.f, 0.01f), 0.f,
            AudioParameterFloatAttributes().withStringFromValueFunction ([](float v,int){
                if (v < -0.005f) return String("L ") + String((int)(-v*100));
                if (v >  0.005f) return String("R ") + String((int)( v*100));
                return String("C"); })));

        p.push_back (std::make_unique<AudioParameterBool> (
            ParameterID { ParamID::bypass, 1 }, "Bypass", false));

        return { p.begin(), p.end() };
    }
}
