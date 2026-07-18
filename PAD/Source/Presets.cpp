#include "Presets.h"
#include "Parameters.h"

namespace Presets
{
    using V = std::vector<std::pair<juce::String, float>>;
    const std::vector<Preset>& getFactoryPresets()
    {
        static const std::vector<Preset> presets = {
            { "Living Pad",  V{ { ParamID::bright, 0.5f }, { ParamID::motion, 0.6f }, { ParamID::shimmer, 0.4f } } },
            { "Dark Choir",  V{ { ParamID::bright, 0.25f }, { ParamID::motion, 0.4f }, { ParamID::shimmer, 0.6f }, { ParamID::odd, 0.7f }, { ParamID::attack, 2.f }, { ParamID::release, 5.f } } },
            { "Bright Halo", V{ { ParamID::bright, 0.85f }, { ParamID::motion, 0.7f }, { ParamID::shimmer, 0.3f }, { ParamID::odd, 0.3f } } },
            { "Slow Breath", V{ { ParamID::bright, 0.4f }, { ParamID::motion, 0.9f }, { ParamID::shimmer, 0.5f }, { ParamID::attack, 4.f }, { ParamID::release, 8.f } } },
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
