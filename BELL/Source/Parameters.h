#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

namespace ParamID
{
    static constexpr auto material = "material";  // Bell / Bar / Glass / Membrane
    static constexpr auto decay    = "decay";
    static constexpr auto bright   = "bright";    // spectral tilt of partials
    static constexpr auto strike   = "strike";    // noise-click level
    static constexpr auto inharm   = "inharm";    // extra ratio stretch
    static constexpr auto space   = "space";    // reverb send
    static constexpr auto drive    = "drive";
    static constexpr auto volume   = "volume";
}

namespace Params
{
    inline juce::AudioProcessorValueTreeState::ParameterLayout createLayout()
    {
        using namespace juce;
        std::vector<std::unique_ptr<RangedAudioParameter>> p;
        auto pct = [](float v,int){ return String ((int)(v*100)) + " %"; };
        auto f = [&](const char* id, const char* nm, float lo, float hi, float def, std::function<String(float,int)> fmt)
        { p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { id, 1 }, nm, NormalisableRange<float>(lo,hi,0.001f), def, AudioParameterFloatAttributes().withStringFromValueFunction (fmt))); };

        p.push_back (std::make_unique<AudioParameterChoice> (ParameterID { ParamID::material, 1 }, "Material",
            StringArray { "Bell", "Bar", "Glass", "Membrane" }, 0));
        f (ParamID::decay, "Decay", 0.f, 1.f, 0.5f, pct);
        f (ParamID::bright, "Brightness", 0.f, 1.f, 0.5f, pct);
        f (ParamID::strike, "Strike", 0.f, 1.f, 0.3f, pct);
        f (ParamID::inharm, "Inharmonic", 0.f, 1.f, 0.3f, pct);
        f (ParamID::space, "Space", 0.f, 1.f, 0.35f, pct);
        f (ParamID::drive, "Drive", 0.f, 1.f, 0.1f, pct);
        p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { ParamID::volume, 1 }, "Volume",
            NormalisableRange<float> (-60.f, 6.f, 0.1f, 2.5f), -8.f, AudioParameterFloatAttributes().withStringFromValueFunction ([](float v,int){return String(v,1)+" dB";})));
        return { p.begin(), p.end() };
    }
}
