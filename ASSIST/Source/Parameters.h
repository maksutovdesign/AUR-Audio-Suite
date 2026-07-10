#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

namespace ParamID
{
    static constexpr auto target    = "target";     // 0 -14, 1 -12, 2 -9, 3 -7
    static constexpr auto tone      = "tone";       // 0 Warm, 1 Neutral, 2 Bright
    static constexpr auto intensity = "intensity";
    static constexpr auto ceiling   = "ceiling";
    static constexpr auto bypass    = "bypass";
}

namespace Params
{
    inline float targetLufs (int choice)
    {
        switch (choice) { case 1: return -12.f; case 2: return -9.f; case 3: return -7.f; default: return -14.f; }
    }
    inline float targetTilt (int choice) // desired (high - low) dB
    {
        switch (choice) { case 0: return -3.f; case 2: return 3.f; default: return 0.f; }
    }

    inline juce::AudioProcessorValueTreeState::ParameterLayout createLayout()
    {
        using namespace juce;
        std::vector<std::unique_ptr<RangedAudioParameter>> p;

        p.push_back (std::make_unique<AudioParameterChoice> (
            ParameterID { ParamID::target, 1 }, "Target",
            StringArray { "-14 Streaming", "-12 Balanced", "-9 Loud", "-7 Club" }, 0));

        p.push_back (std::make_unique<AudioParameterChoice> (
            ParameterID { ParamID::tone, 1 }, "Tone",
            StringArray { "Warm", "Neutral", "Bright" }, 1));

        p.push_back (std::make_unique<AudioParameterFloat> (
            ParameterID { ParamID::intensity, 1 }, "Intensity",
            NormalisableRange<float> (0.f, 100.f, 0.1f), 100.f,
            AudioParameterFloatAttributes().withStringFromValueFunction ([](float v,int){return String((int)v)+"%";})));

        p.push_back (std::make_unique<AudioParameterFloat> (
            ParameterID { ParamID::ceiling, 1 }, "Ceiling",
            NormalisableRange<float> (-3.f, 0.f, 0.1f), -1.f,
            AudioParameterFloatAttributes().withStringFromValueFunction ([](float v,int){return String(v,1)+" dBTP";})));

        p.push_back (std::make_unique<AudioParameterBool> (
            ParameterID { ParamID::bypass, 1 }, "Bypass", false));

        return { p.begin(), p.end() };
    }
}
