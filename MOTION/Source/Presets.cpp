#include "Presets.h"
#include "Parameters.h"

namespace Presets
{
    const std::vector<Preset>& getFactoryPresets()
    {
        static const std::vector<Preset> presets =
        {
            { "Vocal Tame",  { { ParamID::f1, 250.f }, { ParamID::t1, -22.f }, { ParamID::r1, 5.f }, { ParamID::f2, 2500.f }, { ParamID::t2, -20.f }, { ParamID::r2, 6.f }, { ParamID::f3, 7000.f }, { ParamID::t3, -24.f }, { ParamID::r3, 7.f }, { ParamID::q, 2.5f } } },
            { "De-Mud",      { { ParamID::f1, 300.f }, { ParamID::t1, -26.f }, { ParamID::r1, 8.f }, { ParamID::f2, 500.f }, { ParamID::t2, -24.f }, { ParamID::r2, 5.f }, { ParamID::f3, 8000.f }, { ParamID::t3, -30.f }, { ParamID::r3, 3.f }, { ParamID::q, 2.f } } },
            { "Harsh Control",{ { ParamID::f1, 200.f }, { ParamID::t1, -40.f }, { ParamID::r1, 0.f }, { ParamID::f2, 3000.f }, { ParamID::t2, -22.f }, { ParamID::r2, 8.f }, { ParamID::f3, 6000.f }, { ParamID::t3, -22.f }, { ParamID::r3, 8.f }, { ParamID::q, 3.f } } },
            { "Bus Smooth",  { { ParamID::f1, 180.f }, { ParamID::t1, -20.f }, { ParamID::r1, 4.f }, { ParamID::f2, 1200.f }, { ParamID::t2, -22.f }, { ParamID::r2, 4.f }, { ParamID::f3, 9000.f }, { ParamID::t3, -24.f }, { ParamID::r3, 5.f }, { ParamID::q, 1.8f } } },
            { "Gentle",      { { ParamID::f1, 220.f }, { ParamID::t1, -18.f }, { ParamID::r1, 3.f }, { ParamID::f2, 1500.f }, { ParamID::t2, -18.f }, { ParamID::r2, 3.f }, { ParamID::f3, 7000.f }, { ParamID::t3, -20.f }, { ParamID::r3, 3.f }, { ParamID::q, 1.5f } } },
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
