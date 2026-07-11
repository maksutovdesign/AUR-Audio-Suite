#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

namespace ParamID
{
    static constexpr auto character = "character";
    static constexpr auto flavor    = "flavor";
    static constexpr auto input     = "input";
    static constexpr auto hpf        = "hpf";
    static constexpr auto tone       = "tone";
    static constexpr auto output     = "output";
    static constexpr auto bypass     = "bypass";
}

namespace Params
{
    inline juce::AudioProcessorValueTreeState::ParameterLayout createLayout()
    {
        using namespace juce;
        std::vector<std::unique_ptr<RangedAudioParameter>> p;
        auto db = [](float v,int){ return String(v,1)+" dB"; };

        p.push_back (std::make_unique<AudioParameterFloat> (
            ParameterID { ParamID::character, 1 }, "Character",
            NormalisableRange<float> (0.f, 100.f, 0.1f), 40.f,
            AudioParameterFloatAttributes().withStringFromValueFunction ([](float v,int){return String((int)v);})));

        p.push_back (std::make_unique<AudioParameterChoice> (
            ParameterID { ParamID::flavor, 1 }, "Flavor",
            StringArray { "Tube", "Tape", "Iron" }, 0));

        p.push_back (std::make_unique<AudioParameterFloat> (
            ParameterID { ParamID::input, 1 }, "Input",
            NormalisableRange<float> (-24.f, 24.f, 0.01f), 0.f,
            AudioParameterFloatAttributes().withStringFromValueFunction (db)));

        p.push_back (std::make_unique<AudioParameterFloat> (
            ParameterID { ParamID::hpf, 1 }, "HPF",
            NormalisableRange<float> (20.f, 300.f, 1.f, 0.5f), 20.f,
            AudioParameterFloatAttributes().withStringFromValueFunction ([](float v,int){ return v <= 21.f ? String("Off") : String((int)v)+" Hz"; })));

        p.push_back (std::make_unique<AudioParameterFloat> (
            ParameterID { ParamID::tone, 1 }, "Tone",
            NormalisableRange<float> (-100.f, 100.f, 0.1f), 0.f,
            AudioParameterFloatAttributes().withStringFromValueFunction ([](float v,int){
                if (v < -0.5f) return String("Dark ")  + String((int)-v);
                if (v >  0.5f) return String("Bright ") + String((int) v);
                return String("Flat"); })));

        p.push_back (std::make_unique<AudioParameterFloat> (
            ParameterID { ParamID::output, 1 }, "Output",
            NormalisableRange<float> (-24.f, 12.f, 0.01f), 0.f,
            AudioParameterFloatAttributes().withStringFromValueFunction (db)));

        p.push_back (std::make_unique<AudioParameterBool> (
            ParameterID { ParamID::bypass, 1 }, "Bypass", false));

        return { p.begin(), p.end() };
    }
}
