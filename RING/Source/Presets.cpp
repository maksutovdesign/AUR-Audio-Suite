#include "Presets.h"
#include "Parameters.h"

namespace Presets
{
    const std::vector<Preset>& getFactoryPresets()
    {
        static const std::vector<Preset> presets =
        {
            //                        freq     mix    output
            { "Slow Tremolo",   { { ParamID::freq, 4.0f },    { ParamID::mix, 100.f }, { ParamID::output, 0.f } } },
            { "Classic Ring",   { { ParamID::freq, 220.0f },  { ParamID::mix, 100.f }, { ParamID::output, 0.f } } },
            { "Robot Voice",    { { ParamID::freq, 90.0f },   { ParamID::mix, 90.f },  { ParamID::output, 1.f } } },
            { "Metallic Bell",  { { ParamID::freq, 700.0f },  { ParamID::mix, 70.f },  { ParamID::output, -2.f } } },
            { "Clangorous",     { { ParamID::freq, 1500.0f }, { ParamID::mix, 60.f },  { ParamID::output, -3.f } } },
            { "Sci-Fi Sweep",   { { ParamID::freq, 45.0f },   { ParamID::mix, 100.f }, { ParamID::output, 0.f } } },
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
