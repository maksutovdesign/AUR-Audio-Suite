#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <array>

// Seven drum voices, each mapped to a General-MIDI drum note.
namespace Drum
{
    enum { Kick, Snare, Clap, ClosedHat, OpenHat, Tom, Rim, Count };
    static constexpr std::array<int, Count> midiNote { 36, 38, 39, 42, 46, 45, 37 };
    static constexpr std::array<const char*, Count> shortName { "KICK", "SNARE", "CLAP", "C.HAT", "O.HAT", "TOM", "RIM" };
    static constexpr std::array<const char*, Count> idStem { "kick", "snare", "clap", "chat", "ohat", "tom", "rim" };
}

namespace ParamID
{
    static constexpr auto drive  = "drive";
    static constexpr auto volume = "volume";
    // per-voice ids are built as "<stem>_level" etc. — see id() helper.
    inline juce::String level (int v) { return juce::String (Drum::idStem[(size_t) v]) + "_level"; }
    inline juce::String tune  (int v) { return juce::String (Drum::idStem[(size_t) v]) + "_tune"; }
    inline juce::String decay (int v) { return juce::String (Drum::idStem[(size_t) v]) + "_decay"; }
    inline juce::String step (int v, int st) { return juce::String (Drum::idStem[(size_t) v]) + "_s" + juce::String (st); }
    static constexpr auto seqon = "seqon";
}

namespace Params
{
    inline juce::AudioProcessorValueTreeState::ParameterLayout createLayout()
    {
        using namespace juce;
        std::vector<std::unique_ptr<RangedAudioParameter>> p;
        auto pct = [](float v,int){ return String ((int)(v*100)) + " %"; };

        for (int v = 0; v < Drum::Count; ++v)
        {
            const String nm = Drum::shortName[(size_t) v];
            p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { ParamID::level (v), 1 }, nm + " Level",
                NormalisableRange<float> (0.f, 1.f, 0.001f), 0.8f, AudioParameterFloatAttributes().withStringFromValueFunction (pct)));
            p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { ParamID::tune (v), 1 }, nm + " Tune",
                NormalisableRange<float> (0.f, 1.f, 0.001f), 0.5f, AudioParameterFloatAttributes().withStringFromValueFunction (pct)));
            p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { ParamID::decay (v), 1 }, nm + " Decay",
                NormalisableRange<float> (0.f, 1.f, 0.001f), 0.5f, AudioParameterFloatAttributes().withStringFromValueFunction (pct)));
        }

        for (int v = 0; v < Drum::Count; ++v)
            for (int st = 0; st < 16; ++st)
                p.push_back (std::make_unique<AudioParameterBool> (ParameterID { ParamID::step (v, st), 1 },
                    String (Drum::shortName[(size_t) v]) + " S" + String (st + 1), false));
        p.push_back (std::make_unique<AudioParameterBool> (ParameterID { ParamID::seqon, 1 }, "Seq On", true));

        p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { ParamID::drive, 1 }, "Drive",
            NormalisableRange<float> (0.f, 1.f, 0.001f), 0.12f, AudioParameterFloatAttributes().withStringFromValueFunction (pct)));
        p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { ParamID::volume, 1 }, "Volume",
            NormalisableRange<float> (-60.f, 6.f, 0.1f, 2.5f), -6.f, AudioParameterFloatAttributes().withStringFromValueFunction ([](float v,int){return String(v,1)+" dB";})));

        return { p.begin(), p.end() };
    }
}
