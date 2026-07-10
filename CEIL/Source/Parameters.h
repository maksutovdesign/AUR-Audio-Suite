#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

namespace ParamID
{
    static constexpr auto gain    = "gain";     // input drive into the limiter
    static constexpr auto ceiling = "ceiling";  // dBTP
    static constexpr auto release = "release";  // ms
    static constexpr auto bypass  = "bypass";
}

namespace Params
{
    inline juce::AudioProcessorValueTreeState::ParameterLayout createLayout()
    {
        using namespace juce;
        std::vector<std::unique_ptr<RangedAudioParameter>> p;
        auto db = [](float v){ return String (v, 1) + " dB"; };

        p.push_back (std::make_unique<AudioParameterFloat> (
            ParameterID { ParamID::gain, 1 }, "Gain",
            NormalisableRange<float> (0.f, 24.f, 0.01f), 0.f,
            AudioParameterFloatAttributes().withStringFromValueFunction ([db](float v,int){return db(v);})));

        p.push_back (std::make_unique<AudioParameterFloat> (
            ParameterID { ParamID::ceiling, 1 }, "Ceiling",
            NormalisableRange<float> (-12.f, 0.f, 0.1f), -1.f,
            AudioParameterFloatAttributes().withStringFromValueFunction ([](float v,int){return String(v,1)+" dBTP";})));

        p.push_back (std::make_unique<AudioParameterFloat> (
            ParameterID { ParamID::release, 1 }, "Release",
            NormalisableRange<float> (1.f, 1000.f, 1.f, 0.4f), 100.f,
            AudioParameterFloatAttributes().withLabel ("ms")));

        p.push_back (std::make_unique<AudioParameterBool> (
            ParameterID { ParamID::bypass, 1 }, "Bypass", false));

        return { p.begin(), p.end() };
    }
}
