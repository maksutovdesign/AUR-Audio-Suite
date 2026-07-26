#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

namespace ParamID
{
    // Oscillators
    static constexpr auto osc1shape = "osc1shape";
    static constexpr auto osc1pw    = "osc1pw";
    static constexpr auto osc1level = "osc1level";
    static constexpr auto osc2shape = "osc2shape";
    static constexpr auto osc2pw    = "osc2pw";
    static constexpr auto osc2level = "osc2level";
    static constexpr auto osc2coarse= "osc2coarse";
    static constexpr auto osc2fine  = "osc2fine";
    static constexpr auto sublevel  = "sublevel";
    static constexpr auto noiselevel= "noiselevel";
    // Filter
    static constexpr auto cutoff    = "cutoff";
    static constexpr auto resonance = "resonance";
    static constexpr auto fdrive    = "fdrive";
    static constexpr auto envamt    = "envamt";
    static constexpr auto keytrack  = "keytrack";
    // Filter envelope
    static constexpr auto fatk = "fatk"; static constexpr auto fdec = "fdec";
    static constexpr auto fsus = "fsus"; static constexpr auto frel = "frel";
    // Amp envelope
    static constexpr auto aatk = "aatk"; static constexpr auto adec = "adec";
    static constexpr auto asus = "asus"; static constexpr auto arel = "arel";
    // Unison
    static constexpr auto unison = "unison";
    static constexpr auto detune = "detune";
    static constexpr auto spread = "spread";
    // LFO
    static constexpr auto lforate   = "lforate";
    static constexpr auto lfoshape  = "lfoshape";
    static constexpr auto lfo2cut   = "lfo2cut";
    static constexpr auto lfo2pitch = "lfo2pitch";
    // Global
    static constexpr auto drive  = "drive";
    static constexpr auto glide  = "glide";
    static constexpr auto arpon  = "arpon";
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

        const StringArray shapes { "Saw", "Pulse", "Triangle", "Sine" };
        auto pct  = [](float v,int){ return String ((int) v) + " %"; };
        auto sec  = [](float v,int){ return v < 1.f ? String ((int)(v*1000)) + " ms" : String (v,2) + " s"; };
        auto add  = [&p](std::unique_ptr<RangedAudioParameter> x){ p.push_back (std::move (x)); };
        auto flt  = [](const char* id, const char* nm, float lo, float hi, float def, float skew, std::function<String(float,int)> fmt)
        {
            return std::make_unique<AudioParameterFloat> (ParameterID { id, 1 }, nm,
                NormalisableRange<float> (lo, hi, 0.0001f, skew), def,
                AudioParameterFloatAttributes().withStringFromValueFunction (fmt));
        };
        auto lvl  = [&](const char* id, const char* nm, float def){ return flt (id, nm, 0.f, 1.f, def, 1.f, [](float v,int){return String((int)(v*100))+" %";}); };

        // Oscillators
        add (std::make_unique<AudioParameterChoice> (ParameterID { ParamID::osc1shape, 1 }, "Osc1 Shape", shapes, 0));
        add (flt (ParamID::osc1pw, "Osc1 PW", 0.05f, 0.95f, 0.5f, 1.f, pct));
        add (lvl (ParamID::osc1level, "Osc1 Level", 0.8f));
        add (std::make_unique<AudioParameterChoice> (ParameterID { ParamID::osc2shape, 1 }, "Osc2 Shape", shapes, 0));
        add (flt (ParamID::osc2pw, "Osc2 PW", 0.05f, 0.95f, 0.5f, 1.f, pct));
        add (lvl (ParamID::osc2level, "Osc2 Level", 0.0f));
        add (std::make_unique<AudioParameterInt> (ParameterID { ParamID::osc2coarse, 1 }, "Osc2 Coarse", -24, 24, 0));
        add (flt (ParamID::osc2fine, "Osc2 Fine", -100.f, 100.f, 0.f, 1.f, [](float v,int){return String((int)v)+" ct";}));
        add (lvl (ParamID::sublevel,  "Sub Level",  0.0f));
        add (lvl (ParamID::noiselevel,"Noise Level",0.0f));

        // Filter
        add (flt (ParamID::cutoff, "Cutoff", 20.f, 20000.f, 12000.f, 0.28f, [](float v,int){return v<1000.f?String((int)v)+" Hz":String(v/1000.f,2)+" kHz";}));
        add (lvl (ParamID::resonance, "Resonance", 0.15f));
        add (flt (ParamID::fdrive, "Filter Drive", 1.f, 10.f, 1.f, 1.f, [](float v,int){return String(v,1)+"x";}));
        add (flt (ParamID::envamt, "Env Amount", -1.f, 1.f, 0.4f, 1.f, [](float v,int){return String((int)(v*100))+" %";}));
        add (lvl (ParamID::keytrack, "Key Track", 0.25f));

        // Filter envelope
        add (flt (ParamID::fatk, "F.Attack",  0.001f, 5.f, 0.01f, 0.3f, sec));
        add (flt (ParamID::fdec, "F.Decay",   0.001f, 5.f, 0.3f,  0.3f, sec));
        add (lvl (ParamID::fsus, "F.Sustain", 0.3f));
        add (flt (ParamID::frel, "F.Release", 0.001f, 8.f, 0.4f,  0.3f, sec));

        // Amp envelope
        add (flt (ParamID::aatk, "A.Attack",  0.001f, 5.f, 0.005f, 0.3f, sec));
        add (flt (ParamID::adec, "A.Decay",   0.001f, 5.f, 0.4f,   0.3f, sec));
        add (lvl (ParamID::asus, "A.Sustain", 0.8f));
        add (flt (ParamID::arel, "A.Release", 0.001f, 8.f, 0.3f,   0.3f, sec));

        // Unison
        add (std::make_unique<AudioParameterInt> (ParameterID { ParamID::unison, 1 }, "Unison", 1, 7, 1));
        add (lvl (ParamID::detune, "Detune", 0.25f));
        add (lvl (ParamID::spread, "Spread", 0.6f));

        // LFO
        add (flt (ParamID::lforate, "LFO Rate", 0.01f, 20.f, 4.f, 0.35f, [](float v,int){return String(v,2)+" Hz";}));
        add (std::make_unique<AudioParameterChoice> (ParameterID { ParamID::lfoshape, 1 }, "LFO Shape", StringArray { "Sine", "Triangle", "Saw", "Square", "S&H" }, 0));
        add (flt (ParamID::lfo2cut,   "LFO→Cutoff", -1.f, 1.f, 0.f, 1.f, [](float v,int){return String((int)(v*100))+" %";}));
        add (flt (ParamID::lfo2pitch, "LFO→Pitch",  -1.f, 1.f, 0.f, 1.f, [](float v,int){return String((int)(v*100))+" %";}));

        // Global
        add (lvl (ParamID::drive, "Drive", 0.15f));
        add (flt (ParamID::glide, "Glide", 0.f, 1.f, 0.f, 1.f, [](float v,int){return String((int)(v*100))+" %";}));
        add (flt (ParamID::volume, "Volume", -60.f, 6.f, -8.f, 2.5f, [](float v,int){return String(v,1)+" dB";}));

        p.push_back (std::make_unique<AudioParameterBool> (ParameterID { ParamID::arpon, 1 }, "Arp", false));
        p.push_back (std::make_unique<AudioParameterChoice> (ParameterID { ParamID::arpmode, 1 }, "Arp Mode", StringArray { "Up","Down","Up-Down","Random","As Played" }, 0));
        p.push_back (std::make_unique<AudioParameterChoice> (ParameterID { ParamID::arprate, 1 }, "Arp Rate", StringArray { "1/4","1/8","1/16","1/32","1/8T","1/16T" }, 2));
        p.push_back (std::make_unique<AudioParameterInt> (ParameterID { ParamID::arpoct, 1 }, "Arp Octaves", 1, 4, 1));
        p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { ParamID::arpgate, 1 }, "Arp Gate", NormalisableRange<float>(0.05f,1.f,0.001f), 0.7f, AudioParameterFloatAttributes().withStringFromValueFunction([](float v,int){return String((int)(v*100))+" %";})));
        return { p.begin(), p.end() };
    }
}
