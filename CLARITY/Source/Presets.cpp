#include "Presets.h"
#include "Parameters.h"

namespace Presets
{
    const std::vector<Preset>& getFactoryPresets()
    {
        static const std::vector<Preset> presets =
        {
            { "Vocal De-Harsh", { { ParamID::depth, 60.f }, { ParamID::sens, 55.f }, { ParamID::sharpness, 65.f }, { ParamID::mix, 100.f } } },
            { "Gentle Polish",  { { ParamID::depth, 35.f }, { ParamID::sens, 40.f }, { ParamID::sharpness, 45.f }, { ParamID::mix, 100.f } } },
            { "Tame Cymbals",   { { ParamID::depth, 70.f }, { ParamID::sens, 60.f }, { ParamID::sharpness, 75.f }, { ParamID::mix, 90.f } } },
            { "Mix Clarify",    { { ParamID::depth, 45.f }, { ParamID::sens, 50.f }, { ParamID::sharpness, 40.f }, { ParamID::mix, 100.f } } },
            { "Surgical",       { { ParamID::depth, 90.f }, { ParamID::sens, 75.f }, { ParamID::sharpness, 90.f }, { ParamID::mix, 100.f } } },
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
