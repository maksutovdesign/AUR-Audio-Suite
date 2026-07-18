#include "Presets.h"
#include "Parameters.h"

namespace Presets
{
    using V = std::vector<std::pair<juce::String, float>>;
    const std::vector<Preset>& getFactoryPresets()
    {
        static const std::vector<Preset> presets = {
            { "Round Bass",  V{ { ParamID::shape, 0 }, { ParamID::sub, 0.5f }, { ParamID::cutoff, 900.f }, { ParamID::reso, 0.25f }, { ParamID::envamt, 0.5f }, { ParamID::attack, 0.003f }, { ParamID::decay, 0.25f }, { ParamID::sustain, 0.6f }, { ParamID::release, 0.15f }, { ParamID::drive, 0.25f } } },
            { "Bright Lead", V{ { ParamID::shape, 0 }, { ParamID::sub, 0.2f }, { ParamID::cutoff, 6000.f }, { ParamID::reso, 0.2f }, { ParamID::envamt, 0.3f }, { ParamID::attack, 0.005f }, { ParamID::decay, 0.4f }, { ParamID::sustain, 0.8f }, { ParamID::release, 0.3f }, { ParamID::glide, 0.15f } } },
            { "Soft Key",    V{ { ParamID::shape, 2 }, { ParamID::sub, 0.1f }, { ParamID::cutoff, 3500.f }, { ParamID::reso, 0.1f }, { ParamID::envamt, 0.35f }, { ParamID::attack, 0.01f }, { ParamID::decay, 0.6f }, { ParamID::sustain, 0.4f }, { ParamID::release, 0.5f } } },
            { "Acid",        V{ { ParamID::shape, 1 }, { ParamID::pw, 0.4f }, { ParamID::cutoff, 700.f }, { ParamID::reso, 0.8f }, { ParamID::envamt, 0.7f }, { ParamID::keytrack, 0.4f }, { ParamID::attack, 0.002f }, { ParamID::decay, 0.2f }, { ParamID::sustain, 0.2f }, { ParamID::release, 0.1f }, { ParamID::drive, 0.4f } } },
            { "Sine Sub",    V{ { ParamID::shape, 3 }, { ParamID::sub, 0.7f }, { ParamID::cutoff, 1500.f }, { ParamID::reso, 0.f }, { ParamID::envamt, 0.f }, { ParamID::attack, 0.005f }, { ParamID::decay, 0.3f }, { ParamID::sustain, 0.9f }, { ParamID::release, 0.2f }, { ParamID::drive, 0.3f } } },
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
