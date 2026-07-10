#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

namespace ParamID
{
    static constexpr auto hpFreq   = "hpFreq";
    static constexpr auto lsFreq   = "lsFreq";
    static constexpr auto lsGain   = "lsGain";
    static constexpr auto bellFreq = "bellFreq";
    static constexpr auto bellGain = "bellGain";
    static constexpr auto bellQ    = "bellQ";
    static constexpr auto hsFreq   = "hsFreq";
    static constexpr auto hsGain   = "hsGain";
    static constexpr auto lpFreq   = "lpFreq";
    static constexpr auto bypass   = "bypass";
}

namespace Params
{
    inline juce::AudioProcessorValueTreeState::ParameterLayout createLayout()
    {
        using namespace juce;
        std::vector<std::unique_ptr<RangedAudioParameter>> p;

        auto hz  = [](float v,int){ return v >= 1000.f ? String (v/1000.f,1)+" kHz" : String((int)v)+" Hz"; };
        auto dbF = [](float v,int){ return String (v,1)+" dB"; };
        auto freq = [&hz] (const char* id, const char* nm, float lo, float hi, float def)
        {
            return std::make_unique<AudioParameterFloat> (
                ParameterID { id, 1 }, nm, NormalisableRange<float> (lo, hi, 1.f, 0.3f), def,
                AudioParameterFloatAttributes().withStringFromValueFunction (hz));
        };
        auto gain = [&dbF] (const char* id, const char* nm)
        {
            return std::make_unique<AudioParameterFloat> (
                ParameterID { id, 1 }, nm, NormalisableRange<float> (-15.f, 15.f, 0.1f), 0.f,
                AudioParameterFloatAttributes().withStringFromValueFunction (dbF));
        };

        p.push_back (freq (ParamID::hpFreq,   "HP Freq",   20.f, 500.f,   20.f));
        p.push_back (freq (ParamID::lsFreq,   "Low Freq",  30.f, 500.f,   120.f));
        p.push_back (gain (ParamID::lsGain,   "Low Gain"));
        p.push_back (freq (ParamID::bellFreq, "Mid Freq",  100.f, 8000.f, 1000.f));
        p.push_back (gain (ParamID::bellGain, "Mid Gain"));
        p.push_back (std::make_unique<AudioParameterFloat> (
            ParameterID { ParamID::bellQ, 1 }, "Mid Q",
            NormalisableRange<float> (0.3f, 8.f, 0.01f, 0.5f), 1.0f));
        p.push_back (freq (ParamID::hsFreq,   "High Freq", 2000.f, 16000.f, 8000.f));
        p.push_back (gain (ParamID::hsGain,   "High Gain"));
        p.push_back (freq (ParamID::lpFreq,   "LP Freq",   2000.f, 20000.f, 20000.f));

        p.push_back (std::make_unique<AudioParameterBool> (
            ParameterID { ParamID::bypass, 1 }, "Bypass", false));

        return { p.begin(), p.end() };
    }
}
