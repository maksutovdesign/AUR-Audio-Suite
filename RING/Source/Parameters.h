#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

namespace ParamID
{
    static constexpr auto freq   = "freq";   // carrier frequency (Hz)
    static constexpr auto mix    = "mix";    // dry/wet (%)
    static constexpr auto output = "output"; // output trim (dB)
    static constexpr auto bypass = "bypass";
}

namespace Params
{
    inline juce::AudioProcessorValueTreeState::ParameterLayout createLayout()
    {
        using namespace juce;
        std::vector<std::unique_ptr<RangedAudioParameter>> p;

        p.push_back (std::make_unique<AudioParameterFloat> (
            ParameterID { ParamID::freq, 1 }, "Frequency",
            NormalisableRange<float> (1.0f, 5000.0f, 0.01f, 0.3f), 220.0f,
            AudioParameterFloatAttributes().withStringFromValueFunction (
                [](float v,int){ return v < 1000.0f ? String (v,1) + " Hz"
                                                     : String (v/1000.0f,2) + " kHz"; })));

        p.push_back (std::make_unique<AudioParameterFloat> (
            ParameterID { ParamID::mix, 1 }, "Mix",
            NormalisableRange<float> (0.f, 100.f, 0.1f), 100.f,
            AudioParameterFloatAttributes().withStringFromValueFunction ([](float v,int){return String((int)v)+" %";})));

        p.push_back (std::make_unique<AudioParameterFloat> (
            ParameterID { ParamID::output, 1 }, "Output",
            NormalisableRange<float> (-24.f, 12.f, 0.1f), 0.f,
            AudioParameterFloatAttributes().withStringFromValueFunction ([](float v,int){return String(v,1)+" dB";})));

        p.push_back (std::make_unique<AudioParameterBool> (
            ParameterID { ParamID::bypass, 1 }, "Bypass", false));

        return { p.begin(), p.end() };
    }
}
