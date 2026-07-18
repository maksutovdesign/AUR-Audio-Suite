#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

namespace ParamID
{
    static constexpr auto shape   = "shape";    // Pulse12 / Pulse25 / Square / Triangle / Noise
    static constexpr auto crush   = "crush";    // bit-depth reduce
    static constexpr auto vibrato = "vibrato";  // classic chip vibrato depth
    static constexpr auto attack  = "attack";
    static constexpr auto decay   = "decay";
    static constexpr auto sustain = "sustain";
    static constexpr auto release = "release";
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

        p.push_back (std::make_unique<AudioParameterChoice> (ParameterID { ParamID::shape, 1 }, "Shape",
            StringArray { "Pulse 12.5%", "Pulse 25%", "Square", "Triangle", "Noise" }, 2));
        f (ParamID::crush, "Crush", 0.f, 1.f, 0.5f, 1.f, pct);
        f (ParamID::vibrato, "Vibrato", 0.f, 1.f, 0.f, 1.f, pct);
        f (ParamID::attack, "Attack", 0.001f, 1.f, 0.001f, 0.3f, sec);
        f (ParamID::decay, "Decay", 0.001f, 2.f, 0.15f, 0.3f, sec);
        f (ParamID::sustain, "Sustain", 0.f, 1.f, 0.6f, 1.f, pct);
        f (ParamID::release, "Release", 0.001f, 2.f, 0.05f, 0.3f, sec);
        f (ParamID::volume, "Volume", -60.f, 6.f, -10.f, 2.5f, [](float v,int){return String(v,1)+" dB";});
        return { p.begin(), p.end() };
    }
}
