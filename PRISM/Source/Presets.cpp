#include "Presets.h"
#include "Parameters.h"

namespace Presets
{
    const std::vector<Preset>& getFactoryPresets()
    {
        static const std::vector<Preset> presets =
        {
            { "Flat",         { { ParamID::hpFreq, 20.f }, { ParamID::lsGain, 0.f }, { ParamID::bellGain, 0.f }, { ParamID::hsGain, 0.f }, { ParamID::lpFreq, 20000.f } } },
            { "Vocal Air",    { { ParamID::hpFreq, 90.f }, { ParamID::lsFreq, 120.f }, { ParamID::lsGain, -1.5f }, { ParamID::bellFreq, 3000.f }, { ParamID::bellGain, 2.f }, { ParamID::bellQ, 1.2f }, { ParamID::hsFreq, 10000.f }, { ParamID::hsGain, 3.5f } } },
            { "Warm Body",    { { ParamID::hpFreq, 40.f }, { ParamID::lsFreq, 180.f }, { ParamID::lsGain, 2.5f }, { ParamID::bellFreq, 500.f }, { ParamID::bellGain, -2.f }, { ParamID::bellQ, 1.5f }, { ParamID::hsGain, 1.f } } },
            { "De-Mud",       { { ParamID::hpFreq, 60.f }, { ParamID::bellFreq, 300.f }, { ParamID::bellGain, -3.5f }, { ParamID::bellQ, 1.8f } } },
            { "Bright Master",{ { ParamID::lsFreq, 60.f }, { ParamID::lsGain, 1.f }, { ParamID::hsFreq, 12000.f }, { ParamID::hsGain, 2.f }, { ParamID::lpFreq, 20000.f } } },
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
