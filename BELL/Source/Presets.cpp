#include "Presets.h"
#include "Parameters.h"

namespace Presets
{
    using V = std::vector<std::pair<juce::String, float>>;
    const std::vector<Preset>& getFactoryPresets()
    {
        static const std::vector<Preset> presets = {
            { "Church Bell", V{ { ParamID::material, 0 }, { ParamID::decay, 0.8f }, { ParamID::bright, 0.5f }, { ParamID::strike, 0.4f } } },
            { "Marimba",     V{ { ParamID::material, 1 }, { ParamID::decay, 0.25f }, { ParamID::bright, 0.4f }, { ParamID::strike, 0.5f } } },
            { "Wine Glass",  V{ { ParamID::material, 2 }, { ParamID::decay, 0.7f }, { ParamID::bright, 0.6f }, { ParamID::strike, 0.1f } } },
            { "Hand Drum",   V{ { ParamID::material, 3 }, { ParamID::decay, 0.3f }, { ParamID::bright, 0.3f }, { ParamID::strike, 0.6f } } },
            { "Glass Pad",   V{ { ParamID::material, 2 }, { ParamID::decay, 0.95f }, { ParamID::bright, 0.45f }, { ParamID::strike, 0.05f }, { ParamID::inharm, 0.1f } } },
            { "Gamelan",     V{ { ParamID::material, 0 }, { ParamID::decay, 0.55f }, { ParamID::bright, 0.7f }, { ParamID::strike, 0.5f }, { ParamID::inharm, 0.6f }, { ParamID::drive, 0.2f } } },
            { "Music Box",   V{ { ParamID::material, 1 }, { ParamID::decay, 0.4f }, { ParamID::bright, 0.8f }, { ParamID::strike, 0.3f } } },
            { "Deep Gong",   V{ { ParamID::material, 0 }, { ParamID::decay, 1.f }, { ParamID::bright, 0.2f }, { ParamID::strike, 0.6f }, { ParamID::inharm, 0.8f } } },
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
