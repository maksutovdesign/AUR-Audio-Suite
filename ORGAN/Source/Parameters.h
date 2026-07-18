#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

namespace ParamID
{
    inline juce::String bar (int i) { return "bar" + juce::String (i + 1); }   // 9 drawbars
    static constexpr auto perc    = "perc";     // percussion (2nd/3rd click)
    static constexpr auto vibrato = "vibrato";  // scanner vibrato depth
    static constexpr auto rotary  = "rotary";   // rotary speed (Hz)
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
        auto f = [&](const juce::String& id, const juce::String& nm, float lo, float hi, float def, std::function<String(float,int)> fmt)
        { p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { id, 1 }, nm, NormalisableRange<float>(lo,hi,0.001f), def, AudioParameterFloatAttributes().withStringFromValueFunction (fmt))); };

        static const char* names[9] = { "16'", "5 1/3'", "8'", "4'", "2 2/3'", "2'", "1 3/5'", "1 1/3'", "1'" };
        static const float defs[9]  = { 0.9f, 0.f, 0.9f, 0.7f, 0.f, 0.4f, 0.f, 0.f, 0.2f };
        for (int i = 0; i < 9; ++i) f (ParamID::bar (i), names[i], 0.f, 1.f, defs[i], pct);
        f (ParamID::perc, "Percussion", 0.f, 1.f, 0.3f, pct);
        f (ParamID::vibrato, "Vibrato", 0.f, 1.f, 0.2f, pct);
        f (ParamID::rotary, "Rotary", 0.f, 1.f, 0.4f, pct);
        f (ParamID::drive, "Drive", 0.f, 1.f, 0.25f, pct);
        p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { ParamID::volume, 1 }, "Volume",
            NormalisableRange<float> (-60.f, 6.f, 0.1f, 2.5f), -10.f, AudioParameterFloatAttributes().withStringFromValueFunction ([](float v,int){return String(v,1)+" dB";})));
        return { p.begin(), p.end() };
    }
}
