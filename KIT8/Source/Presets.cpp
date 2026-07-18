#include "Presets.h"
#include "Parameters.h"

namespace Presets
{
    using V = std::vector<std::pair<juce::String, float>>;
    const std::vector<Preset>& getFactoryPresets()
    {
        static const std::vector<Preset> presets = {
            { "Classic 808",  V{ { ParamID::sweep, 0.5f }, { ParamID::punch, 0.4f }, { ParamID::decay, 1.2f }, { ParamID::drive, 0.45f } } },
            { "Long Boom",    V{ { ParamID::sweep, 0.4f }, { ParamID::punch, 0.3f }, { ParamID::decay, 3.f }, { ParamID::tone, 1200.f }, { ParamID::drive, 0.5f } } },
            { "Hard Knock",   V{ { ParamID::sweep, 0.9f }, { ParamID::punch, 0.8f }, { ParamID::decay, 0.6f }, { ParamID::tone, 4000.f }, { ParamID::drive, 0.6f } } },
            { "Smooth Slide", V{ { ParamID::sweep, 0.3f }, { ParamID::punch, 0.2f }, { ParamID::decay, 2.f }, { ParamID::glide, 0.7f }, { ParamID::drive, 0.4f } } },
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
