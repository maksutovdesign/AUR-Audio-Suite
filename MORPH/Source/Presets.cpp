#include "Presets.h"
#include "Parameters.h"

namespace Presets
{
    using V = std::vector<std::pair<juce::String, float>>;
    const std::vector<Preset>& getFactoryPresets()
    {
        static const std::vector<Preset> presets = {
            { "VA Start",     V{ { ParamID::morph, 0.05f }, { ParamID::motion, 0.2f } } },
            { "FM Zone",      V{ { ParamID::morph, 0.4f }, { ParamID::motion, 0.1f }, { ParamID::drive, 0.3f } } },
            { "Slow Traveler",V{ { ParamID::morph, 0.5f }, { ParamID::motion, 1.f }, { ParamID::rate, 0.08f }, { ParamID::attack, 0.8f }, { ParamID::release, 2.f } } },
            { "Organ End",    V{ { ParamID::morph, 0.95f }, { ParamID::motion, 0.1f } } },
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
