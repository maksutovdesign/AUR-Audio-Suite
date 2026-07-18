#include "Presets.h"
#include "Parameters.h"

namespace Presets
{
    using V = std::vector<std::pair<juce::String, float>>;
    const std::vector<Preset>& getFactoryPresets()
    {
        static const std::vector<Preset> presets = {
            { "Hyper Lead",   V{ { ParamID::morph1, 0.7f }, { ParamID::unison, 5 }, { ParamID::detune, 0.45f }, { ParamID::spread, 0.9f }, { ParamID::drive, 0.3f } } },
            { "FM Growl",     V{ { ParamID::morph1, 0.9f }, { ParamID::fm, 0.6f }, { ParamID::o2coarse, -12.f }, { ParamID::o2level, 0.3f }, { ParamID::cutoff, 2500.f }, { ParamID::reso, 0.4f }, { ParamID::envamt, 0.6f }, { ParamID::drive, 0.4f } } },
            { "Morphing Pad", V{ { ParamID::morph1, 0.3f }, { ParamID::lfo2morph, 0.6f }, { ParamID::lforate, 0.2f }, { ParamID::unison, 4 }, { ParamID::aatk, 0.8f }, { ParamID::arel, 2.f } } },
            { "Acid Hyper",   V{ { ParamID::morph1, 0.66f }, { ParamID::cutoff, 600.f }, { ParamID::reso, 0.75f }, { ParamID::envamt, 0.8f }, { ParamID::fdec, 0.18f }, { ParamID::fsus, 0.f }, { ParamID::glide, 0.3f }, { ParamID::drive, 0.45f } } },
            { "BP Texture",   V{ { ParamID::morph1, 0.5f }, { ParamID::fmode, 1 }, { ParamID::lfo2cut, 0.5f }, { ParamID::lforate, 4.f }, { ParamID::reso, 0.5f } } },
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
