#include "Presets.h"
#include "Parameters.h"

namespace Presets
{
    const std::vector<Preset>& getFactoryPresets()
    {
        static const std::vector<Preset> presets =
        {
            { "Transparent",   { { ParamID::gain, 0.f },  { ParamID::ceiling, -1.0f }, { ParamID::release, 120.f } } },
            { "Streaming -1",  { { ParamID::gain, 3.f },  { ParamID::ceiling, -1.0f }, { ParamID::release, 100.f } } },
            { "Loud Master",   { { ParamID::gain, 8.f },  { ParamID::ceiling, -0.3f }, { ParamID::release, 60.f } } },
            { "Gentle Catch",  { { ParamID::gain, 1.f },  { ParamID::ceiling, -0.5f }, { ParamID::release, 200.f } } },
            { "Brickwall",     { { ParamID::gain, 12.f }, { ParamID::ceiling, -0.1f }, { ParamID::release, 40.f } } },
        };
        return presets;
    }

    void apply (int index, juce::AudioProcessorValueTreeState& apvts)
    {
        const auto& p = getFactoryPresets();
        if (index < 0 || index >= (int) p.size()) return;
        for (const auto& [id, value] : p[(size_t) index].values)
            if (auto* param = apvts.getParameter (id))
                param->setValueNotifyingHost (param->convertTo0to1 (value));
    }
}
