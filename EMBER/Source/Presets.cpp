#include "Presets.h"
#include "Parameters.h"

namespace Presets
{
    const std::vector<Preset>& getFactoryPresets()
    {
        static const std::vector<Preset> presets =
        {
            { "Warm Vocal",   { { ParamID::flavor, 0.f }, { ParamID::drive, 22.f }, { ParamID::mix, 55.f }, { ParamID::tone, 12.f }, { ParamID::outputGain, 0.f } } },
            { "Tape Glue",    { { ParamID::flavor, 1.f }, { ParamID::drive, 30.f }, { ParamID::mix, 100.f }, { ParamID::tone, -8.f }, { ParamID::outputGain, 0.f } } },
            { "Iron Punch",   { { ParamID::flavor, 2.f }, { ParamID::drive, 45.f }, { ParamID::mix, 70.f }, { ParamID::tone, 6.f }, { ParamID::outputGain, -1.f } } },
            { "Bus Warmth",   { { ParamID::flavor, 1.f }, { ParamID::drive, 15.f }, { ParamID::mix, 40.f }, { ParamID::tone, 0.f }, { ParamID::outputGain, 0.f } } },
            { "Lo-Fi Melt",   { { ParamID::flavor, 2.f }, { ParamID::drive, 80.f }, { ParamID::mix, 100.f }, { ParamID::tone, -30.f }, { ParamID::outputGain, -2.f } } },
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
