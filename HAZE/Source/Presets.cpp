#include "Presets.h"
#include "Parameters.h"

namespace Presets
{
    const std::vector<Preset>& getFactoryPresets()
    {
        static const std::vector<Preset> presets =
        {
            { "Vocal Plate",  { { ParamID::size, 45.f }, { ParamID::decay, 1.8f }, { ParamID::damp, 45.f }, { ParamID::predelay, 25.f }, { ParamID::width, 110.f }, { ParamID::mix, 25.f } } },
            { "Small Room",   { { ParamID::size, 25.f }, { ParamID::decay, 0.8f }, { ParamID::damp, 55.f }, { ParamID::predelay, 8.f }, { ParamID::width, 90.f }, { ParamID::mix, 20.f } } },
            { "Big Hall",     { { ParamID::size, 85.f }, { ParamID::decay, 4.5f }, { ParamID::damp, 35.f }, { ParamID::predelay, 40.f }, { ParamID::width, 130.f }, { ParamID::mix, 35.f } } },
            { "Ambient Wash", { { ParamID::size, 95.f }, { ParamID::decay, 8.f }, { ParamID::damp, 25.f }, { ParamID::predelay, 60.f }, { ParamID::width, 150.f }, { ParamID::mix, 50.f } } },
            { "Tight Drums",  { { ParamID::size, 30.f }, { ParamID::decay, 0.9f }, { ParamID::damp, 60.f }, { ParamID::predelay, 5.f }, { ParamID::width, 100.f }, { ParamID::mix, 18.f } } },
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
