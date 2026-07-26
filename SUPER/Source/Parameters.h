#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

namespace ParamID
{
    static constexpr auto detune  = "detune";
    static constexpr auto spread  = "spread";
    static constexpr auto mixOsc  = "mixosc";   // centre vs side saws
    static constexpr auto cutoff  = "cutoff";
    static constexpr auto reso    = "reso";
    static constexpr auto attack  = "attack";
    static constexpr auto decay   = "decay";
    static constexpr auto sustain = "sustain";
    static constexpr auto release = "release";
    static constexpr auto glide   = "glide";
    static constexpr auto drive   = "drive";
    static constexpr auto arpon  = "arpon";
    static constexpr auto arpmode= "arpmode";
    static constexpr auto arprate= "arprate";
    static constexpr auto arpoct = "arpoct";
    static constexpr auto arpgate= "arpgate";
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

        f (ParamID::detune, "Detune", 0.f, 1.f, 0.4f, 1.f, pct);
        f (ParamID::spread, "Spread", 0.f, 1.f, 0.8f, 1.f, pct);
        f (ParamID::mixOsc, "Mix", 0.f, 1.f, 0.7f, 1.f, pct);
        f (ParamID::cutoff, "Cutoff", 20.f, 20000.f, 9000.f, 0.28f, [](float v,int){return v<1000.f?String((int)v)+" Hz":String(v/1000.f,2)+" kHz";});
        f (ParamID::reso, "Resonance", 0.f, 1.f, 0.15f, 1.f, pct);
        f (ParamID::attack, "Attack", 0.001f, 5.f, 0.01f, 0.3f, sec);
        f (ParamID::decay, "Decay", 0.001f, 5.f, 0.5f, 0.3f, sec);
        f (ParamID::sustain, "Sustain", 0.f, 1.f, 0.8f, 1.f, pct);
        f (ParamID::release, "Release", 0.001f, 8.f, 0.4f, 0.3f, sec);
        f (ParamID::glide, "Glide", 0.f, 1.f, 0.f, 1.f, pct);
        f (ParamID::drive, "Drive", 0.f, 1.f, 0.2f, 1.f, pct);
        f (ParamID::volume, "Volume", -60.f, 6.f, -10.f, 2.5f, [](float v,int){return String(v,1)+" dB";});
        p.push_back (std::make_unique<AudioParameterBool> (ParameterID { ParamID::arpon, 1 }, "Arp", false));
        p.push_back (std::make_unique<AudioParameterChoice> (ParameterID { ParamID::arpmode, 1 }, "Arp Mode", StringArray { "Up","Down","Up-Down","Random","As Played" }, 0));
        p.push_back (std::make_unique<AudioParameterChoice> (ParameterID { ParamID::arprate, 1 }, "Arp Rate", StringArray { "1/4","1/8","1/16","1/32","1/8T","1/16T" }, 2));
        p.push_back (std::make_unique<AudioParameterInt> (ParameterID { ParamID::arpoct, 1 }, "Arp Octaves", 1, 4, 1));
        p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { ParamID::arpgate, 1 }, "Arp Gate", NormalisableRange<float>(0.05f,1.f,0.001f), 0.7f, AudioParameterFloatAttributes().withStringFromValueFunction([](float v,int){return String((int)(v*100))+" %";})));
        return { p.begin(), p.end() };
    }
}
