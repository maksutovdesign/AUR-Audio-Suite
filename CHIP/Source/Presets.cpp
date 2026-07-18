#include "Presets.h"
#include "Parameters.h"

namespace Presets
{
    using V = std::vector<std::pair<juce::String, float>>;
    const std::vector<Preset>& getFactoryPresets()
    {
        static const std::vector<Preset> presets = {
            { "Hero Lead",   V{ { ParamID::shape, 2 }, { ParamID::crush, 0.4f }, { ParamID::vibrato, 0.3f } } },
            { "Thin Pulse",  V{ { ParamID::shape, 0 }, { ParamID::crush, 0.5f }, { ParamID::decay, 0.1f } } },
            { "Tri Bass",    V{ { ParamID::shape, 3 }, { ParamID::crush, 0.6f }, { ParamID::sustain, 0.9f } } },
            { "Noise Hit",   V{ { ParamID::shape, 4 }, { ParamID::crush, 0.7f }, { ParamID::decay, 0.08f }, { ParamID::sustain, 0.f } } },
            { "Arcade Blip", V{ { ParamID::shape, 1 }, { ParamID::crush, 0.8f }, { ParamID::decay, 0.05f }, { ParamID::sustain, 0.f }, { ParamID::release, 0.02f } } },
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
