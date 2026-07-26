#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

namespace ParamID
{
    static constexpr auto morph1 = "morph1";   static constexpr auto morph2 = "morph2";
    static constexpr auto o2coarse = "o2coarse"; static constexpr auto o2level = "o2level";
    static constexpr auto fm = "fm";
    static constexpr auto unison = "unison";   static constexpr auto detune = "detune";  static constexpr auto spread = "spread";
    static constexpr auto cutoff = "cutoff";   static constexpr auto reso = "reso";      static constexpr auto fmode = "fmode";
    static constexpr auto envamt = "envamt";
    static constexpr auto fatk = "fatk"; static constexpr auto fdec = "fdec"; static constexpr auto fsus = "fsus"; static constexpr auto frel = "frel";
    static constexpr auto aatk = "aatk"; static constexpr auto adec = "adec"; static constexpr auto asus = "asus"; static constexpr auto arel = "arel";
    static constexpr auto lforate = "lforate"; static constexpr auto lfo2cut = "lfo2cut"; static constexpr auto lfo2morph = "lfo2morph";
    static constexpr auto glide = "glide"; static constexpr auto drive = "drive"; static constexpr auto arpon  = "arpon";
    static constexpr auto arpmode= "arpmode";
    static constexpr auto arprate= "arprate";
    static constexpr auto arpoct = "arpoct";
    static constexpr auto arpgate= "arpgate";
    static constexpr auto volume = "volume";
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

        f (ParamID::morph1, "Osc1 Morph", 0.f, 1.f, 0.66f, 1.f, pct);
        f (ParamID::morph2, "Osc2 Morph", 0.f, 1.f, 0.33f, 1.f, pct);
        p.push_back (std::make_unique<AudioParameterInt> (ParameterID { ParamID::o2coarse, 1 }, "Osc2 Coarse", -24, 24, -12));
        f (ParamID::o2level, "Osc2 Level", 0.f, 1.f, 0.4f, 1.f, pct);
        f (ParamID::fm, "FM 2→1", 0.f, 1.f, 0.f, 1.f, pct);
        p.push_back (std::make_unique<AudioParameterInt> (ParameterID { ParamID::unison, 1 }, "Unison", 1, 5, 3));
        f (ParamID::detune, "Detune", 0.f, 1.f, 0.3f, 1.f, pct);
        f (ParamID::spread, "Spread", 0.f, 1.f, 0.7f, 1.f, pct);
        f (ParamID::cutoff, "Cutoff", 20.f, 20000.f, 9000.f, 0.28f, [](float v,int){return v<1000.f?String((int)v)+" Hz":String(v/1000.f,2)+" kHz";});
        f (ParamID::reso, "Resonance", 0.f, 1.f, 0.15f, 1.f, pct);
        p.push_back (std::make_unique<AudioParameterChoice> (ParameterID { ParamID::fmode, 1 }, "Filter", StringArray { "LP", "BP", "HP" }, 0));
        f (ParamID::envamt, "Env Amt", -1.f, 1.f, 0.35f, 1.f, [](float v,int){return String((int)(v*100))+" %";});
        f (ParamID::fatk, "F.Atk", 0.001f, 5.f, 0.01f, 0.3f, sec);
        f (ParamID::fdec, "F.Dec", 0.001f, 5.f, 0.35f, 0.3f, sec);
        f (ParamID::fsus, "F.Sus", 0.f, 1.f, 0.3f, 1.f, pct);
        f (ParamID::frel, "F.Rel", 0.001f, 8.f, 0.4f, 0.3f, sec);
        f (ParamID::aatk, "A.Atk", 0.001f, 5.f, 0.005f, 0.3f, sec);
        f (ParamID::adec, "A.Dec", 0.001f, 5.f, 0.4f, 0.3f, sec);
        f (ParamID::asus, "A.Sus", 0.f, 1.f, 0.8f, 1.f, pct);
        f (ParamID::arel, "A.Rel", 0.001f, 8.f, 0.35f, 0.3f, sec);
        f (ParamID::lforate, "LFO Rate", 0.02f, 15.f, 1.f, 0.35f, [](float v,int){return String(v,2)+" Hz";});
        f (ParamID::lfo2cut, "LFO→Cut", -1.f, 1.f, 0.f, 1.f, [](float v,int){return String((int)(v*100))+" %";});
        f (ParamID::lfo2morph, "LFO→Morph", -1.f, 1.f, 0.f, 1.f, [](float v,int){return String((int)(v*100))+" %";});
        f (ParamID::glide, "Glide", 0.f, 1.f, 0.f, 1.f, pct);
        f (ParamID::drive, "Drive", 0.f, 1.f, 0.2f, 1.f, pct);
        f (ParamID::volume, "Volume", -60.f, 6.f, -9.f, 2.5f, [](float v,int){return String(v,1)+" dB";});
        p.push_back (std::make_unique<AudioParameterBool> (ParameterID { ParamID::arpon, 1 }, "Arp", false));
        p.push_back (std::make_unique<AudioParameterChoice> (ParameterID { ParamID::arpmode, 1 }, "Arp Mode", StringArray { "Up","Down","Up-Down","Random","As Played" }, 0));
        p.push_back (std::make_unique<AudioParameterChoice> (ParameterID { ParamID::arprate, 1 }, "Arp Rate", StringArray { "1/4","1/8","1/16","1/32","1/8T","1/16T" }, 2));
        p.push_back (std::make_unique<AudioParameterInt> (ParameterID { ParamID::arpoct, 1 }, "Arp Octaves", 1, 4, 1));
        p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { ParamID::arpgate, 1 }, "Arp Gate", NormalisableRange<float>(0.05f,1.f,0.001f), 0.7f, AudioParameterFloatAttributes().withStringFromValueFunction([](float v,int){return String((int)(v*100))+" %";})));
        return { p.begin(), p.end() };
    }
}
