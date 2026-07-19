#include "Presets.h"
#include "Parameters.h"

namespace Presets
{
    using V = std::vector<std::pair<juce::String, float>>;
    const std::vector<Preset>& getFactoryPresets()
    {
        static const std::vector<Preset> presets = {
            { "Moving Saw",  V{ { ParamID::morph, 0.66f }, { ParamID::lfoamt, 0.25f }, { ParamID::lforate, 0.5f }, { ParamID::detune, 0.35f } } },
            { "Glass Morph", V{ { ParamID::morph, 0.2f }, { ParamID::lfoamt, 0.6f }, { ParamID::lforate, 0.15f }, { ParamID::attack, 0.4f }, { ParamID::release, 1.2f } } },
            { "Square Bite", V{ { ParamID::morph, 1.f }, { ParamID::lfoamt, 0.1f }, { ParamID::cutoff, 4000.f }, { ParamID::envamt, 0.6f }, { ParamID::drive, 0.3f } } },
            { "Wobble",      V{ { ParamID::morph, 0.5f }, { ParamID::lfoamt, 0.9f }, { ParamID::lforate, 5.f }, { ParamID::cutoff, 2500.f }, { ParamID::reso, 0.4f } } },
            { "Vox Morph",   V{ { ParamID::morph, 0.35f }, { ParamID::lfoamt, 0.4f }, { ParamID::lforate, 0.8f }, { ParamID::cutoff, 3500.f }, { ParamID::reso, 0.5f }, { ParamID::detune, 0.2f } } },
            { "Deep Table",  V{ { ParamID::morph, 0.8f }, { ParamID::lfoamt, 0.15f }, { ParamID::cutoff, 1200.f }, { ParamID::envamt, 0.5f }, { ParamID::drive, 0.35f } } },
            { "Air Sine",    V{ { ParamID::morph, 0.05f }, { ParamID::lfoamt, 0.3f }, { ParamID::lforate, 0.2f }, { ParamID::attack, 0.6f }, { ParamID::release, 2.f } } },
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
