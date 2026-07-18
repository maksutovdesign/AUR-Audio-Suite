#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

namespace ParamID
{
    static constexpr auto sweep  = "sweep";   // pitch-drop amount
    static constexpr auto punch  = "punch";   // click level
    static constexpr auto decay  = "decay";   // tail length
    static constexpr auto tone   = "tone";    // LP cutoff
    static constexpr auto glide  = "glide";
    static constexpr auto drive  = "drive";
    static constexpr auto volume = "volume";
}

namespace Params
{
    inline juce::AudioProcessorValueTreeState::ParameterLayout createLayout()
    {
        using namespace juce;
        std::vector<std::unique_ptr<RangedAudioParameter>> p;
        auto pct = [](float v,int){ return String ((int)(v*100)) + " %"; };
        auto f = [&](const char* id, const char* nm, float lo, float hi, float def, float sk, std::function<String(float,int)> fmt)
        { p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { id, 1 }, nm, NormalisableRange<float>(lo,hi,0.0001f,sk), def, AudioParameterFloatAttributes().withStringFromValueFunction (fmt))); };

        f (ParamID::sweep, "Sweep", 0.f, 1.f, 0.5f, 1.f, pct);
        f (ParamID::punch, "Punch", 0.f, 1.f, 0.4f, 1.f, pct);
        f (ParamID::decay, "Decay", 0.1f, 4.f, 1.2f, 0.4f, [](float v,int){return String(v,2)+" s";});
        f (ParamID::tone, "Tone", 200.f, 8000.f, 2000.f, 0.3f, [](float v,int){return v<1000.f?String((int)v)+" Hz":String(v/1000.f,2)+" kHz";});
        f (ParamID::glide, "Glide", 0.f, 1.f, 0.3f, 1.f, pct);
        f (ParamID::drive, "Drive", 0.f, 1.f, 0.45f, 1.f, pct);
        f (ParamID::volume, "Volume", -60.f, 6.f, -5.f, 2.5f, [](float v,int){return String(v,1)+" dB";});
        return { p.begin(), p.end() };
    }
}
