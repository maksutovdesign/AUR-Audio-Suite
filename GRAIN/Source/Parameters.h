#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

namespace ParamID
{
    static constexpr auto density = "density";  // grains per second
    static constexpr auto size    = "size";     // grain length
    static constexpr auto spray   = "spray";    // random pitch per grain (cents)
    static constexpr auto texture = "texture";  // source wavetable morph
    static constexpr auto attack  = "attack";
    static constexpr auto release = "release";
    static constexpr auto space   = "space";    // reverb send
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

        f (ParamID::density, "Density", 2.f, 80.f, 25.f, 0.4f, [](float v,int){return String((int)v)+" /s";});
        f (ParamID::size, "Size", 0.02f, 0.3f, 0.09f, 0.5f, sec);
        f (ParamID::spray, "Spray", 0.f, 1.f, 0.3f, 1.f, pct);
        f (ParamID::texture, "Texture", 0.f, 1.f, 0.5f, 1.f, pct);
        f (ParamID::attack, "Attack", 0.01f, 5.f, 0.3f, 0.3f, sec);
        f (ParamID::release, "Release", 0.01f, 8.f, 1.f, 0.3f, sec);
        f (ParamID::space, "Space", 0.f, 1.f, 0.35f, 1.f, pct);
        f (ParamID::drive, "Drive", 0.f, 1.f, 0.1f, 1.f, pct);
        f (ParamID::volume, "Volume", -60.f, 6.f, -9.f, 2.5f, [](float v,int){return String(v,1)+" dB";});
        return { p.begin(), p.end() };
    }
}
