#include "Presets.h"
#include "Parameters.h"

namespace Presets
{
    using V = std::vector<std::pair<juce::String, float>>;
    const std::vector<Preset>& getFactoryPresets()
    {
        static const std::vector<Preset> presets = {
            { "Trance Lead",  V{ { ParamID::detune, 0.45f }, { ParamID::spread, 0.9f }, { ParamID::mixOsc, 0.75f }, { ParamID::cutoff, 10000.f }, { ParamID::drive, 0.25f } } },
            { "Hyper Wide",   V{ { ParamID::detune, 0.8f }, { ParamID::spread, 1.f }, { ParamID::mixOsc, 0.9f }, { ParamID::cutoff, 14000.f }, { ParamID::drive, 0.35f } } },
            { "Soft Saw Pad", V{ { ParamID::detune, 0.3f }, { ParamID::spread, 0.7f }, { ParamID::mixOsc, 0.6f }, { ParamID::cutoff, 3500.f }, { ParamID::attack, 0.5f }, { ParamID::release, 1.5f } } },
            { "Tight Pluck",  V{ { ParamID::detune, 0.35f }, { ParamID::spread, 0.6f }, { ParamID::cutoff, 6000.f }, { ParamID::decay, 0.25f }, { ParamID::sustain, 0.f }, { ParamID::release, 0.2f } } },
            { "Anthem Chord", V{ { ParamID::detune, 0.55f }, { ParamID::spread, 1.f }, { ParamID::mixOsc, 0.85f }, { ParamID::cutoff, 12000.f }, { ParamID::attack, 0.15f }, { ParamID::release, 0.8f }, { ParamID::drive, 0.3f } } },
            { "Dark Unison",  V{ { ParamID::detune, 0.5f }, { ParamID::spread, 0.8f }, { ParamID::cutoff, 1800.f }, { ParamID::reso, 0.3f }, { ParamID::drive, 0.4f } } },
            { "Glass Keys",   V{ { ParamID::detune, 0.2f }, { ParamID::spread, 0.5f }, { ParamID::cutoff, 9000.f }, { ParamID::decay, 0.5f }, { ParamID::sustain, 0.3f }, { ParamID::release, 0.6f } } },
        };
        return presets;
    }
    void apply (int index, juce::AudioProcessorValueTreeState& apvts)
    {
        const auto& p = getFactoryPresets();
        if (index < 0 || index >= (int) p.size()) return;
        for (const auto& [id, value] : p[(size_t) index].values)
            if (auto* param = apvts.getParameter (id)) param->setValueNotifyingHost (param->convertTo0to1 (value));
    }
}
