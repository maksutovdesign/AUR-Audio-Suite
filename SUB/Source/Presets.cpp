#include "Presets.h"
#include "Parameters.h"

namespace Presets
{
    using V = std::vector<std::pair<juce::String, float>>;
    const std::vector<Preset>& getFactoryPresets()
    {
        static const std::vector<Preset> presets = {
            { "Clean Sub",   V{ { ParamID::punch, 0.1f }, { ParamID::harm, 0.1f }, { ParamID::drive, 0.15f } } },
            { "808 Long",    V{ { ParamID::punch, 0.5f }, { ParamID::harm, 0.3f }, { ParamID::decay, 1.5f }, { ParamID::sustain, 0.f }, { ParamID::drive, 0.4f } } },
            { "Knock",       V{ { ParamID::punch, 0.9f }, { ParamID::harm, 0.4f }, { ParamID::decay, 0.3f }, { ParamID::sustain, 0.3f }, { ParamID::drive, 0.5f } } },
            { "Warm Round",  V{ { ParamID::punch, 0.2f }, { ParamID::harm, 0.5f }, { ParamID::drive, 0.35f }, { ParamID::glide, 0.3f } } },
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
