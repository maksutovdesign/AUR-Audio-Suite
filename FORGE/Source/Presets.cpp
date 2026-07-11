#include "Presets.h"
#include "Parameters.h"

namespace Presets
{
    const std::vector<Preset>& getFactoryPresets()
    {
        static const std::vector<Preset> presets =
        {
            { "Vocal · Molten Air", { { ParamID::character, 45.f }, { ParamID::flavor, 0.f }, { ParamID::hpf, 90.f }, { ParamID::tone, 15.f }, { ParamID::output, 0.f } } },
            { "Podcast Warm",       { { ParamID::character, 55.f }, { ParamID::flavor, 1.f }, { ParamID::hpf, 80.f }, { ParamID::tone, -8.f }, { ParamID::output, 0.f } } },
            { "Bus Glue",           { { ParamID::character, 30.f }, { ParamID::flavor, 1.f }, { ParamID::hpf, 20.f }, { ParamID::tone, 0.f }, { ParamID::output, 0.f } } },
            { "Drum Iron",          { { ParamID::character, 65.f }, { ParamID::flavor, 2.f }, { ParamID::hpf, 40.f }, { ParamID::tone, 12.f }, { ParamID::output, -1.f } } },
            { "Master Gentle",      { { ParamID::character, 22.f }, { ParamID::flavor, 0.f }, { ParamID::hpf, 20.f }, { ParamID::tone, 3.f }, { ParamID::output, 0.f } } },
            { "Full Molten",        { { ParamID::character, 90.f }, { ParamID::flavor, 2.f }, { ParamID::hpf, 60.f }, { ParamID::tone, -5.f }, { ParamID::output, -2.f } } },
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
