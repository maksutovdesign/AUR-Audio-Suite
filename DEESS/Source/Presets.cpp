#include "Presets.h"
#include "Parameters.h"

namespace Presets
{
    const std::vector<Preset>& getFactoryPresets()
    {
        static const std::vector<Preset> presets =
        {
            { "Vocal Soft",   { { ParamID::freq, 6500.f }, { ParamID::threshold, -28.f }, { ParamID::range, 8.f } } },
            { "Vocal Strong", { { ParamID::freq, 7000.f }, { ParamID::threshold, -34.f }, { ParamID::range, 14.f } } },
            { "Bright Mix",   { { ParamID::freq, 8000.f }, { ParamID::threshold, -30.f }, { ParamID::range, 6.f } } },
            { "Low Sibilance",{ { ParamID::freq, 5000.f }, { ParamID::threshold, -26.f }, { ParamID::range, 10.f } } },
            { "Gentle",       { { ParamID::freq, 6800.f }, { ParamID::threshold, -22.f }, { ParamID::range, 5.f } } },
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
