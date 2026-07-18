#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

namespace ParamID
{
    static constexpr auto punch   = "punch";    // pitch-envelope click
    static constexpr auto harm    = "harm";     // 2nd-harmonic level
    static constexpr auto attack  = "attack";
    static constexpr auto decay   = "decay";
    static constexpr auto sustain = "sustain";
    static constexpr auto release = "release";
    static constexpr auto glide   = "glide";
    static constexpr auto drive   = "drive";
    static constexpr auto volume  = "volume";
}

namespace Params
{
    inline juce::AudioProcessorValueTreeState::ParameterLayout createLayout()
    {
        using namespace juce;
        std::vector<std::unique_ptr<RangedAudioParameter>> p;
        auto sec = [](float v,int){ return v < 1.f ? String ((int)(v*1000)) + " ms" : String (v,2) + " s"; };
        auto pct = [](float v,int){ return String ((int)(v*100)) + " %"; };
        auto f = [&](const char* id, const char* nm, float lo, float hi, float def, float sk, std::function<String(float,int)> fmt)
        { p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { id, 1 }, nm, NormalisableRange<float>(lo,hi,0.0001f,sk), def, AudioParameterFloatAttributes().withStringFromValueFunction (fmt))); };

        f (ParamID::punch, "Punch", 0.f, 1.f, 0.3f, 1.f, pct);
        f (ParamID::harm, "Harmonic", 0.f, 1.f, 0.25f, 1.f, pct);
        f (ParamID::attack, "Attack", 0.001f, 2.f, 0.003f, 0.3f, sec);
        f (ParamID::decay, "Decay", 0.001f, 5.f, 0.4f, 0.3f, sec);
        f (ParamID::sustain, "Sustain", 0.f, 1.f, 0.8f, 1.f, pct);
        f (ParamID::release, "Release", 0.001f, 5.f, 0.15f, 0.3f, sec);
        f (ParamID::glide, "Glide", 0.f, 1.f, 0.2f, 1.f, pct);
        f (ParamID::drive, "Drive", 0.f, 1.f, 0.3f, 1.f, pct);
        f (ParamID::volume, "Volume", -60.f, 6.f, -6.f, 2.5f, [](float v,int){return String(v,1)+" dB";});
        return { p.begin(), p.end() };
    }
}
