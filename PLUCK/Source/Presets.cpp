#include "Presets.h"
#include "Parameters.h"

namespace Presets
{
    using V = std::vector<std::pair<juce::String, float>>;
    const std::vector<Preset>& getFactoryPresets()
    {
        static const std::vector<Preset> presets = {
            { "Nylon Guitar", V{ { ParamID::damp, 0.5f }, { ParamID::sustain, 0.7f }, { ParamID::bright, 0.5f }, { ParamID::spread, 0.3f }, { ParamID::drive, 0.1f } } },
            { "Bright Harp",  V{ { ParamID::damp, 0.3f }, { ParamID::sustain, 0.85f }, { ParamID::bright, 0.8f }, { ParamID::spread, 0.5f }, { ParamID::drive, 0.05f } } },
            { "Muted Pluck",  V{ { ParamID::damp, 0.8f }, { ParamID::sustain, 0.4f }, { ParamID::bright, 0.4f }, { ParamID::spread, 0.2f }, { ParamID::drive, 0.15f } } },
            { "Koto",         V{ { ParamID::damp, 0.55f }, { ParamID::sustain, 0.75f }, { ParamID::bright, 0.7f }, { ParamID::spread, 0.6f }, { ParamID::drive, 0.2f } } },
            { "Long Sustain", V{ { ParamID::damp, 0.35f }, { ParamID::sustain, 0.98f }, { ParamID::bright, 0.55f }, { ParamID::spread, 0.45f }, { ParamID::drive, 0.1f } } },
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
