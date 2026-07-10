#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

namespace ParamID
{
    static constexpr auto f1 = "f1"; static constexpr auto t1 = "t1"; static constexpr auto r1 = "r1";
    static constexpr auto f2 = "f2"; static constexpr auto t2 = "t2"; static constexpr auto r2 = "r2";
    static constexpr auto f3 = "f3"; static constexpr auto t3 = "t3"; static constexpr auto r3 = "r3";
    static constexpr auto q      = "q";
    static constexpr auto bypass = "bypass";
}

namespace Params
{
    inline juce::AudioProcessorValueTreeState::ParameterLayout createLayout()
    {
        using namespace juce;
        std::vector<std::unique_ptr<RangedAudioParameter>> p;
        auto hz  = [](float v,int){ return v >= 1000.f ? String(v/1000.f,1)+" kHz" : String((int)v)+" Hz"; };
        auto db  = [](float v,int){ return String(v,1)+" dB"; };

        auto freq = [&hz] (const char* id, const char* nm, float def)
        {
            return std::make_unique<AudioParameterFloat> (ParameterID { id, 1 }, nm,
                NormalisableRange<float> (30.f, 16000.f, 1.f, 0.3f), def,
                AudioParameterFloatAttributes().withStringFromValueFunction (hz));
        };
        auto thr = [&db] (const char* id, const char* nm)
        {
            return std::make_unique<AudioParameterFloat> (ParameterID { id, 1 }, nm,
                NormalisableRange<float> (-60.f, 0.f, 0.1f), -24.f,
                AudioParameterFloatAttributes().withStringFromValueFunction (db));
        };
        auto rng = [&db] (const char* id, const char* nm)
        {
            return std::make_unique<AudioParameterFloat> (ParameterID { id, 1 }, nm,
                NormalisableRange<float> (0.f, 18.f, 0.1f), 6.f,
                AudioParameterFloatAttributes().withStringFromValueFunction (db));
        };

        p.push_back (freq (ParamID::f1, "Low Freq",  200.f));
        p.push_back (thr  (ParamID::t1, "Low Thr"));
        p.push_back (rng  (ParamID::r1, "Low Range"));
        p.push_back (freq (ParamID::f2, "Mid Freq",  1000.f));
        p.push_back (thr  (ParamID::t2, "Mid Thr"));
        p.push_back (rng  (ParamID::r2, "Mid Range"));
        p.push_back (freq (ParamID::f3, "High Freq", 6000.f));
        p.push_back (thr  (ParamID::t3, "High Thr"));
        p.push_back (rng  (ParamID::r3, "High Range"));

        p.push_back (std::make_unique<AudioParameterFloat> (
            ParameterID { ParamID::q, 1 }, "Q",
            NormalisableRange<float> (0.5f, 8.f, 0.01f, 0.5f), 2.5f));

        p.push_back (std::make_unique<AudioParameterBool> (
            ParameterID { ParamID::bypass, 1 }, "Bypass", false));

        return { p.begin(), p.end() };
    }
}
