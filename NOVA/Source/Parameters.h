#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

namespace ParamID
{
    // Per-operator ids are built at runtime: "op<i>_ratio" etc. (i = 1..4)
    inline juce::String ratio (int op) { return "op" + juce::String (op + 1) + "_ratio"; }
    inline juce::String level (int op) { return "op" + juce::String (op + 1) + "_level"; }
    inline juce::String atk   (int op) { return "op" + juce::String (op + 1) + "_atk"; }
    inline juce::String dec   (int op) { return "op" + juce::String (op + 1) + "_dec"; }
    inline juce::String sus   (int op) { return "op" + juce::String (op + 1) + "_sus"; }
    inline juce::String rel   (int op) { return "op" + juce::String (op + 1) + "_rel"; }

    static constexpr auto algo     = "algo";
    static constexpr auto feedback = "feedback";
    static constexpr auto glide    = "glide";
    static constexpr auto drive    = "drive";
    static constexpr auto volume   = "volume";
}

namespace Params
{
    inline juce::AudioProcessorValueTreeState::ParameterLayout createLayout()
    {
        using namespace juce;
        std::vector<std::unique_ptr<RangedAudioParameter>> p;
        auto sec = [](float v,int){ return v < 1.f ? String ((int)(v*1000)) + " ms" : String (v,2) + " s"; };
        auto pct = [](float v,int){ return String ((int)(v*100)) + " %"; };

        const float ratioDef[4] = { 1.0f, 1.0f, 2.0f, 3.0f };
        const float lvlDef[4]   = { 0.9f, 0.6f, 0.0f, 0.0f };
        for (int i = 0; i < 4; ++i)
        {
            p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { ParamID::ratio (i), 1 }, "Op" + String (i+1) + " Ratio",
                NormalisableRange<float> (0.25f, 16.f, 0.01f, 0.5f), ratioDef[i],
                AudioParameterFloatAttributes().withStringFromValueFunction ([](float v,int){ return String (v,2) + "×"; })));
            p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { ParamID::level (i), 1 }, "Op" + String (i+1) + " Level",
                NormalisableRange<float> (0.f, 1.f, 0.001f), lvlDef[i], AudioParameterFloatAttributes().withStringFromValueFunction (pct)));
            p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { ParamID::atk (i), 1 }, "Op" + String (i+1) + " Attack",
                NormalisableRange<float> (0.001f, 5.f, 0.0001f, 0.3f), 0.005f, AudioParameterFloatAttributes().withStringFromValueFunction (sec)));
            p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { ParamID::dec (i), 1 }, "Op" + String (i+1) + " Decay",
                NormalisableRange<float> (0.001f, 5.f, 0.0001f, 0.3f), 0.5f, AudioParameterFloatAttributes().withStringFromValueFunction (sec)));
            p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { ParamID::sus (i), 1 }, "Op" + String (i+1) + " Sustain",
                NormalisableRange<float> (0.f, 1.f, 0.001f), 0.7f, AudioParameterFloatAttributes().withStringFromValueFunction (pct)));
            p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { ParamID::rel (i), 1 }, "Op" + String (i+1) + " Release",
                NormalisableRange<float> (0.001f, 8.f, 0.0001f, 0.3f), 0.4f, AudioParameterFloatAttributes().withStringFromValueFunction (sec)));
        }

        p.push_back (std::make_unique<AudioParameterChoice> (ParameterID { ParamID::algo, 1 }, "Algorithm",
            StringArray { "Chain", "Twin Stack", "3→Carrier", "Y-Stack", "Parallel", "Additive" }, 0));
        p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { ParamID::feedback, 1 }, "Feedback",
            NormalisableRange<float> (0.f, 1.f, 0.001f), 0.f, AudioParameterFloatAttributes().withStringFromValueFunction (pct)));
        p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { ParamID::glide, 1 }, "Glide",
            NormalisableRange<float> (0.f, 1.f, 0.001f), 0.f, AudioParameterFloatAttributes().withStringFromValueFunction (pct)));
        p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { ParamID::drive, 1 }, "Drive",
            NormalisableRange<float> (0.f, 1.f, 0.001f), 0.1f, AudioParameterFloatAttributes().withStringFromValueFunction (pct)));
        p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { ParamID::volume, 1 }, "Volume",
            NormalisableRange<float> (-60.f, 6.f, 0.1f, 2.5f), -10.f, AudioParameterFloatAttributes().withStringFromValueFunction ([](float v,int){return String(v,1)+" dB";})));

        return { p.begin(), p.end() };
    }
}
