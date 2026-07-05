#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

namespace ParamID
{
    static constexpr auto inputGain  = "inputGain";
    static constexpr auto flavor     = "flavor";     // 0 Tube, 1 Tape, 2 Iron
    static constexpr auto drive      = "drive";
    static constexpr auto mix        = "mix";
    static constexpr auto tone       = "tone";
    static constexpr auto outputGain = "outputGain";
    static constexpr auto bypass     = "bypass";
}

namespace Params
{
    inline juce::AudioProcessorValueTreeState::ParameterLayout createLayout()
    {
        using namespace juce;
        std::vector<std::unique_ptr<RangedAudioParameter>> p;

        auto db = [](float v){ return String (v, 1) + " dB"; };

        p.push_back (std::make_unique<AudioParameterFloat> (
            ParameterID { ParamID::inputGain, 1 }, "Input",
            NormalisableRange<float> (-24.f, 24.f, 0.01f), 0.f,
            AudioParameterFloatAttributes().withStringFromValueFunction ([db](float v,int){return db(v);})));

        p.push_back (std::make_unique<AudioParameterChoice> (
            ParameterID { ParamID::flavor, 1 }, "Flavor",
            StringArray { "Tube", "Tape", "Iron" }, 0));

        p.push_back (std::make_unique<AudioParameterFloat> (
            ParameterID { ParamID::drive, 1 }, "Drive",
            NormalisableRange<float> (0.f, 100.f, 0.1f), 25.f,
            AudioParameterFloatAttributes().withLabel ("%")));

        p.push_back (std::make_unique<AudioParameterFloat> (
            ParameterID { ParamID::mix, 1 }, "Mix",
            NormalisableRange<float> (0.f, 100.f, 0.1f), 100.f,
            AudioParameterFloatAttributes().withLabel ("%")));

        p.push_back (std::make_unique<AudioParameterFloat> (
            ParameterID { ParamID::tone, 1 }, "Tone",
            NormalisableRange<float> (-100.f, 100.f, 0.1f), 0.f,
            AudioParameterFloatAttributes().withStringFromValueFunction ([](float v,int){
                if (v < -0.5f) return String ("Dark ")   + String ((int) -v);
                if (v >  0.5f) return String ("Bright ")  + String ((int)  v);
                return String ("Flat"); })));

        p.push_back (std::make_unique<AudioParameterFloat> (
            ParameterID { ParamID::outputGain, 1 }, "Output",
            NormalisableRange<float> (-24.f, 12.f, 0.01f), 0.f,
            AudioParameterFloatAttributes().withStringFromValueFunction ([db](float v,int){return db(v);})));

        p.push_back (std::make_unique<AudioParameterBool> (
            ParameterID { ParamID::bypass, 1 }, "Bypass", false));

        return { p.begin(), p.end() };
    }
}
