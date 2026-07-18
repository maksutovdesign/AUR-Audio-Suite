#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

namespace ParamID
{
    static constexpr auto damp    = "damp";      // string damping → tone
    static constexpr auto sustain = "sustain";   // feedback decay → ring time
    static constexpr auto bright  = "bright";    // pluck brightness (noise-burst LP)
    static constexpr auto spread  = "spread";    // stereo detune of the twin strings
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
        auto f = [&](const char* id, const char* nm, float def, std::function<String(float,int)> fmt)
        { p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { id, 1 }, nm, NormalisableRange<float>(0.f,1.f,0.001f), def, AudioParameterFloatAttributes().withStringFromValueFunction (fmt))); };

        f (ParamID::damp,    "Damping",    0.5f, pct);
        f (ParamID::sustain, "Sustain",    0.7f, pct);
        f (ParamID::bright,  "Brightness", 0.6f, pct);
        f (ParamID::spread,  "Spread",     0.4f, pct);
        f (ParamID::drive,   "Drive",      0.1f, pct);
        p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { ParamID::volume, 1 }, "Volume",
            NormalisableRange<float> (-60.f, 6.f, 0.1f, 2.5f), -6.f, AudioParameterFloatAttributes().withStringFromValueFunction ([](float v,int){return String(v,1)+" dB";})));
        return { p.begin(), p.end() };
    }
}
