#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

namespace ParamID
{
    static constexpr auto shape    = "shape";
    static constexpr auto pw       = "pw";
    static constexpr auto sub       = "sub";
    static constexpr auto cutoff    = "cutoff";
    static constexpr auto reso       = "reso";
    static constexpr auto envamt    = "envamt";
    static constexpr auto keytrack  = "keytrack";
    static constexpr auto attack   = "attack";
    static constexpr auto decay    = "decay";
    static constexpr auto sustain  = "sustain";
    static constexpr auto release  = "release";
    static constexpr auto glide    = "glide";
    static constexpr auto drive    = "drive";
    static constexpr auto volume   = "volume";
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

        p.push_back (std::make_unique<AudioParameterChoice> (ParameterID { ParamID::shape, 1 }, "Shape", StringArray { "Saw", "Pulse", "Triangle", "Sine" }, 0));
        f (ParamID::pw, "PW", 0.05f, 0.95f, 0.5f, 1.f, pct);
        f (ParamID::sub, "Sub", 0.f, 1.f, 0.3f, 1.f, pct);
        f (ParamID::cutoff, "Cutoff", 20.f, 20000.f, 3000.f, 0.28f, [](float v,int){return v<1000.f?String((int)v)+" Hz":String(v/1000.f,2)+" kHz";});
        f (ParamID::reso, "Resonance", 0.f, 1.f, 0.2f, 1.f, pct);
        f (ParamID::envamt, "Env Amount", 0.f, 1.f, 0.5f, 1.f, pct);
        f (ParamID::keytrack, "Key Track", 0.f, 1.f, 0.3f, 1.f, pct);
        f (ParamID::attack, "Attack", 0.001f, 5.f, 0.005f, 0.3f, sec);
        f (ParamID::decay, "Decay", 0.001f, 5.f, 0.3f, 0.3f, sec);
        f (ParamID::sustain, "Sustain", 0.f, 1.f, 0.7f, 1.f, pct);
        f (ParamID::release, "Release", 0.001f, 8.f, 0.3f, 0.3f, sec);
        f (ParamID::glide, "Glide", 0.f, 1.f, 0.f, 1.f, pct);
        f (ParamID::drive, "Drive", 0.f, 1.f, 0.15f, 1.f, pct);
        f (ParamID::volume, "Volume", -60.f, 6.f, -8.f, 2.5f, [](float v,int){return String(v,1)+" dB";});

        return { p.begin(), p.end() };
    }
}
