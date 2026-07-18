#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

namespace ParamID
{
    static constexpr auto octave  = "octave";    // octave-layer level
    static constexpr auto ensemble= "ensemble";  // chorus depth
    static constexpr auto rate    = "rate";      // chorus rate
    static constexpr auto tone    = "tone";      // LP tilt
    static constexpr auto attack  = "attack";
    static constexpr auto release = "release";
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

        f (ParamID::octave, "Octave Layer", 0.f, 1.f, 0.4f, 1.f, pct);
        f (ParamID::ensemble, "Ensemble", 0.f, 1.f, 0.7f, 1.f, pct);
        f (ParamID::rate, "Rate", 0.1f, 8.f, 0.8f, 0.4f, [](float v,int){return String(v,2)+" Hz";});
        f (ParamID::tone, "Tone", 500.f, 16000.f, 6000.f, 0.3f, [](float v,int){return v<1000.f?String((int)v)+" Hz":String(v/1000.f,2)+" kHz";});
        f (ParamID::attack, "Attack", 0.01f, 3.f, 0.25f, 0.3f, sec);
        f (ParamID::release, "Release", 0.01f, 5.f, 0.6f, 0.3f, sec);
        f (ParamID::drive, "Drive", 0.f, 1.f, 0.15f, 1.f, pct);
        f (ParamID::volume, "Volume", -60.f, 6.f, -10.f, 2.5f, [](float v,int){return String(v,1)+" dB";});
        return { p.begin(), p.end() };
    }
}
