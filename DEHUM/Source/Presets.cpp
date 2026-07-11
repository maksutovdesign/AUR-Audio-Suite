#include "Presets.h"
#include "Parameters.h"

namespace Presets
{
    const std::vector<Preset>& getFactoryPresets()
    {
        static const std::vector<Preset> presets =
        {
            { "50 Hz (EU)",    { { ParamID::freq, 50.f }, { ParamID::harmonics, 4.f }, { ParamID::depth, 24.f }, { ParamID::q, 20.f } } },
            { "60 Hz (US)",    { { ParamID::freq, 60.f }, { ParamID::harmonics, 4.f }, { ParamID::depth, 24.f }, { ParamID::q, 20.f } } },
            { "Strong 50 Hz",  { { ParamID::freq, 50.f }, { ParamID::harmonics, 6.f }, { ParamID::depth, 40.f }, { ParamID::q, 28.f } } },
            { "Gentle 60 Hz",  { { ParamID::freq, 60.f }, { ParamID::harmonics, 3.f }, { ParamID::depth, 14.f }, { ParamID::q, 16.f } } },
            { "Buzz Kill",     { { ParamID::freq, 50.f }, { ParamID::harmonics, 8.f }, { ParamID::depth, 44.f }, { ParamID::q, 32.f } } },
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
