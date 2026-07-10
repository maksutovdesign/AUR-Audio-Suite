#include "Presets.h"
#include "Parameters.h"

namespace Presets
{
    const std::vector<Preset>& getFactoryPresets()
    {
        static const std::vector<Preset> presets =
        {
            { "Vocal Level",  { { ParamID::threshold, -20.f }, { ParamID::ratio, 3.f }, { ParamID::attack, 8.f }, { ParamID::release, 120.f }, { ParamID::makeup, 4.f }, { ParamID::mix, 100.f } } },
            { "Drum Smack",   { { ParamID::threshold, -16.f }, { ParamID::ratio, 4.f }, { ParamID::attack, 25.f }, { ParamID::release, 140.f }, { ParamID::makeup, 3.f }, { ParamID::mix, 70.f } } },
            { "Bus Glue",     { { ParamID::threshold, -12.f }, { ParamID::ratio, 2.f }, { ParamID::attack, 30.f }, { ParamID::release, 200.f }, { ParamID::makeup, 1.5f }, { ParamID::mix, 100.f } } },
            { "Bass Control", { { ParamID::threshold, -22.f }, { ParamID::ratio, 6.f }, { ParamID::attack, 5.f }, { ParamID::release, 90.f }, { ParamID::makeup, 4.f }, { ParamID::mix, 100.f } } },
            { "Squash",       { { ParamID::threshold, -28.f }, { ParamID::ratio, 12.f }, { ParamID::attack, 3.f }, { ParamID::release, 250.f }, { ParamID::makeup, 6.f }, { ParamID::mix, 100.f } } },
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
