#include "Presets.h"
#include "Parameters.h"

namespace Presets
{
    using V = std::vector<std::pair<juce::String, float>>;
    const std::vector<Preset>& getFactoryPresets()
    {
        static const std::vector<Preset> presets = {
            { "Deep Space",  V{ { ParamID::detune, 0.5f }, { ParamID::motion, 0.6f }, { ParamID::cutoff, 1200.f }, { ParamID::attack, 3.f }, { ParamID::release, 6.f } } },
            { "Warm Bed",    V{ { ParamID::detune, 0.3f }, { ParamID::motion, 0.3f }, { ParamID::cutoff, 2500.f }, { ParamID::drive, 0.3f } } },
            { "Dark Rumble", V{ { ParamID::detune, 0.7f }, { ParamID::motion, 0.8f }, { ParamID::cutoff, 500.f }, { ParamID::reso, 0.4f }, { ParamID::drive, 0.4f } } },
            { "Airy Drift",  V{ { ParamID::detune, 0.4f }, { ParamID::motion, 0.7f }, { ParamID::cutoff, 8000.f }, { ParamID::attack, 5.f }, { ParamID::release, 8.f } } },
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
