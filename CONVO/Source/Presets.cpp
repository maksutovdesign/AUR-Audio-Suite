#include "Presets.h"
#include "Parameters.h"

namespace Presets
{
    const std::vector<Preset>& getFactoryPresets()
    {
        static const std::vector<Preset> presets =
        {
            //                       decay   tone  predly width  mix
            { "Vocal Plate",    { { ParamID::decay, 1.6f }, { ParamID::tone, 62.f }, { ParamID::predelay, 18.f }, { ParamID::width, 100.f }, { ParamID::mix, 28.f } } },
            { "Small Room",     { { ParamID::decay, 0.6f }, { ParamID::tone, 45.f }, { ParamID::predelay, 6.f  }, { ParamID::width, 80.f  }, { ParamID::mix, 22.f } } },
            { "Concert Hall",   { { ParamID::decay, 3.2f }, { ParamID::tone, 55.f }, { ParamID::predelay, 28.f }, { ParamID::width, 100.f }, { ParamID::mix, 34.f } } },
            { "Cathedral",      { { ParamID::decay, 5.5f }, { ParamID::tone, 40.f }, { ParamID::predelay, 45.f }, { ParamID::width, 100.f }, { ParamID::mix, 40.f } } },
            { "Dark Ambient",   { { ParamID::decay, 4.5f }, { ParamID::tone, 18.f }, { ParamID::predelay, 10.f }, { ParamID::width, 100.f }, { ParamID::mix, 55.f } } },
            { "Bright Chamber", { { ParamID::decay, 1.1f }, { ParamID::tone, 85.f }, { ParamID::predelay, 8.f  }, { ParamID::width, 90.f  }, { ParamID::mix, 26.f } } },
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
