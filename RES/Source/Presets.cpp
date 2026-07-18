#include "Presets.h"
#include "Parameters.h"

namespace Presets
{
    using V = std::vector<std::pair<juce::String, float>>;
    const std::vector<Preset>& getFactoryPresets()
    {
        static const std::vector<Preset> presets = {
            { "Bowed Wire",   V{ { ParamID::exciter, 0.8f }, { ParamID::pick, 0.1f }, { ParamID::resfb, 0.9f }, { ParamID::damp, 0.5f }, { ParamID::body, 0.6f } } },
            { "Plucked Tube", V{ { ParamID::exciter, 0.05f }, { ParamID::pick, 0.9f }, { ParamID::resfb, 0.8f }, { ParamID::damp, 0.35f } } },
            { "Breath Pipe",  V{ { ParamID::exciter, 0.9f }, { ParamID::pick, 0.f }, { ParamID::resfb, 0.7f }, { ParamID::damp, 0.6f }, { ParamID::body, 0.8f } } },
            { "Metal Ring",   V{ { ParamID::exciter, 0.3f }, { ParamID::pick, 0.6f }, { ParamID::resfb, 0.98f }, { ParamID::damp, 0.7f }, { ParamID::drive, 0.3f } } },
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
