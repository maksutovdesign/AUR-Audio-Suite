#include "Presets.h"
#include "Parameters.h"

namespace Presets
{
    using V = std::vector<std::pair<juce::String, float>>;
    const std::vector<Preset>& getFactoryPresets()
    {
        static const std::vector<Preset> presets = {
            { "Cloud Pad",   V{ { ParamID::density, 30.f }, { ParamID::size, 0.12f }, { ParamID::spray, 0.25f }, { ParamID::texture, 0.3f } } },
            { "Shimmer",     V{ { ParamID::density, 50.f }, { ParamID::size, 0.05f }, { ParamID::spray, 0.6f }, { ParamID::texture, 0.1f } } },
            { "Rough Swarm", V{ { ParamID::density, 15.f }, { ParamID::size, 0.08f }, { ParamID::spray, 0.8f }, { ParamID::texture, 0.9f }, { ParamID::drive, 0.3f } } },
            { "Slow Dust",   V{ { ParamID::density, 6.f }, { ParamID::size, 0.25f }, { ParamID::spray, 0.4f }, { ParamID::attack, 1.5f }, { ParamID::release, 3.f } } },
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
