#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

namespace ParamID
{
    static constexpr auto exciter = "exciter";  // sustained breath-noise level
    static constexpr auto pick    = "pick";     // initial burst level
    static constexpr auto resfb   = "resfb";    // comb feedback (ring)
    static constexpr auto damp    = "damp";     // brightness of the loop
    static constexpr auto body    = "body";     // bandpass emphasis at f0
    static constexpr auto drive   = "drive";
    static constexpr auto volume  = "volume";
}

namespace Params
{
    inline juce::AudioProcessorValueTreeState::ParameterLayout createLayout()
    {
        using namespace juce;
        std::vector<std::unique_ptr<RangedAudioParameter>> p;
        auto pct = [](float v,int){ return String ((int)(v*100)) + " %"; };
        auto f = [&](const char* id, const char* nm, float def)
        { p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { id, 1 }, nm, NormalisableRange<float>(0.f,1.f,0.001f), def, AudioParameterFloatAttributes().withStringFromValueFunction (pct))); };

        f (ParamID::exciter, "Exciter", 0.5f);
        f (ParamID::pick, "Pick", 0.4f);
        f (ParamID::resfb, "Resonance", 0.85f);
        f (ParamID::damp, "Damping", 0.4f);
        f (ParamID::body, "Body", 0.5f);
        f (ParamID::drive, "Drive", 0.1f);
        p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { ParamID::volume, 1 }, "Volume",
            NormalisableRange<float> (-60.f, 6.f, 0.1f, 2.5f), -8.f, AudioParameterFloatAttributes().withStringFromValueFunction ([](float v,int){return String(v,1)+" dB";})));
        return { p.begin(), p.end() };
    }
}
