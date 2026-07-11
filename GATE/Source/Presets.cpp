#include "Presets.h"
#include "Parameters.h"

namespace Presets
{
    const std::vector<Preset>& getFactoryPresets()
    {
        static const std::vector<Preset> presets =
        {
            { "Vocal Clean",  { { ParamID::threshold, -42.f }, { ParamID::range, 45.f }, { ParamID::attack, 1.f }, { ParamID::hold, 60.f }, { ParamID::release, 150.f } } },
            { "Drum Tight",   { { ParamID::threshold, -30.f }, { ParamID::range, 70.f }, { ParamID::attack, 0.3f }, { ParamID::hold, 20.f }, { ParamID::release, 60.f } } },
            { "Guitar Noise", { { ParamID::threshold, -50.f }, { ParamID::range, 60.f }, { ParamID::attack, 2.f }, { ParamID::hold, 80.f }, { ParamID::release, 200.f } } },
            { "Hard Gate",    { { ParamID::threshold, -36.f }, { ParamID::range, 90.f }, { ParamID::attack, 0.5f }, { ParamID::hold, 10.f }, { ParamID::release, 40.f } } },
            { "Gentle Expand",{ { ParamID::threshold, -45.f }, { ParamID::range, 12.f }, { ParamID::attack, 5.f }, { ParamID::hold, 100.f }, { ParamID::release, 300.f } } },
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
