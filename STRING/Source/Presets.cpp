#include "Presets.h"
#include "Parameters.h"

namespace Presets
{
    using V = std::vector<std::pair<juce::String, float>>;
    const std::vector<Preset>& getFactoryPresets()
    {
        static const std::vector<Preset> presets = {
            { "Solina",       V{ { ParamID::octave, 0.4f }, { ParamID::ensemble, 0.7f }, { ParamID::rate, 0.8f }, { ParamID::tone, 6000.f } } },
            { "Dark Strings", V{ { ParamID::octave, 0.2f }, { ParamID::ensemble, 0.5f }, { ParamID::rate, 0.5f }, { ParamID::tone, 2500.f }, { ParamID::attack, 0.6f }, { ParamID::release, 1.5f } } },
            { "Silky High",   V{ { ParamID::octave, 0.7f }, { ParamID::ensemble, 0.8f }, { ParamID::rate, 1.2f }, { ParamID::tone, 10000.f } } },
            { "Fast Swirl",   V{ { ParamID::octave, 0.4f }, { ParamID::ensemble, 0.9f }, { ParamID::rate, 4.f }, { ParamID::tone, 7000.f } } },
        };
        return presets;
    }
    void apply (int index, juce::AudioProcessorValueTreeState& apvts)
    {
        const auto& p = getFactoryPresets();
        if (index < 0 || index >= (int) p.size()) return;
        for (const auto& [id, value] : p[(size_t) index].values)
            if (auto* param = apvts.getParameter (id)) param->setValueNotifyingHost (param->convertTo0to1 (value));
    }
}
