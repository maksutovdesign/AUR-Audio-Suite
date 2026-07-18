#include "Presets.h"
#include "Parameters.h"

namespace Presets
{
    using V = std::vector<std::pair<juce::String, float>>;
    const std::vector<Preset>& getFactoryPresets()
    {
        static const std::vector<Preset> presets = {
            { "Orbit Pad",   V{ { ParamID::x, 0.5f }, { ParamID::y, 0.5f }, { ParamID::orbit, 0.6f }, { ParamID::rate, 0.3f }, { ParamID::attack, 0.5f }, { ParamID::release, 1.5f } } },
            { "Saw Corner",  V{ { ParamID::x, 0.05f }, { ParamID::y, 0.05f }, { ParamID::orbit, 0.15f }, { ParamID::drive, 0.25f } } },
            { "Sub Swirl",   V{ { ParamID::x, 0.8f }, { ParamID::y, 0.8f }, { ParamID::orbit, 0.5f }, { ParamID::rate, 1.2f }, { ParamID::cutoff, 3000.f } } },
            { "Fast Vector", V{ { ParamID::x, 0.5f }, { ParamID::y, 0.5f }, { ParamID::orbit, 0.9f }, { ParamID::rate, 6.f } } },
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
