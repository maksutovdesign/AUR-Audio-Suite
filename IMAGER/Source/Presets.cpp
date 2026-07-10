#include "Presets.h"
#include "Parameters.h"

namespace Presets
{
    const std::vector<Preset>& getFactoryPresets()
    {
        static const std::vector<Preset> presets =
        {
            { "Natural",       { { ParamID::width, 100.f }, { ParamID::monoBelow, 20.f }, { ParamID::balance, 0.f } } },
            { "Wide Master",   { { ParamID::width, 130.f }, { ParamID::monoBelow, 120.f }, { ParamID::balance, 0.f } } },
            { "Mono Bass",     { { ParamID::width, 110.f }, { ParamID::monoBelow, 200.f }, { ParamID::balance, 0.f } } },
            { "Focus Center",  { { ParamID::width, 70.f },  { ParamID::monoBelow, 20.f },  { ParamID::balance, 0.f } } },
            { "Super Wide",    { { ParamID::width, 180.f }, { ParamID::monoBelow, 150.f }, { ParamID::balance, 0.f } } },
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
