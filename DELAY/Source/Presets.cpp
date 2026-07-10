#include "Presets.h"
#include "Parameters.h"

namespace Presets
{
    const std::vector<Preset>& getFactoryPresets()
    {
        static const std::vector<Preset> presets =
        {
            { "Slap",       { { ParamID::time, 110.f }, { ParamID::feedback, 12.f }, { ParamID::damp, 40.f }, { ParamID::width, 100.f }, { ParamID::pingpong, 0.f }, { ParamID::mix, 22.f } } },
            { "Quarter",    { { ParamID::time, 500.f }, { ParamID::feedback, 40.f }, { ParamID::damp, 45.f }, { ParamID::width, 110.f }, { ParamID::pingpong, 0.f }, { ParamID::mix, 30.f } } },
            { "Ping-Pong",  { { ParamID::time, 375.f }, { ParamID::feedback, 55.f }, { ParamID::damp, 50.f }, { ParamID::width, 140.f }, { ParamID::pingpong, 1.f }, { ParamID::mix, 35.f } } },
            { "Dub Echo",   { { ParamID::time, 600.f }, { ParamID::feedback, 75.f }, { ParamID::damp, 70.f }, { ParamID::width, 120.f }, { ParamID::pingpong, 1.f }, { ParamID::mix, 40.f } } },
            { "Ambient",    { { ParamID::time, 900.f }, { ParamID::feedback, 60.f }, { ParamID::damp, 35.f }, { ParamID::width, 150.f }, { ParamID::pingpong, 0.f }, { ParamID::mix, 45.f } } },
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
