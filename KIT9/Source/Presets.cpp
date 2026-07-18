#include "Presets.h"
#include "Parameters.h"

namespace Presets
{
    using V = std::vector<std::pair<juce::String, float>>;

    // Helper to set a voice's tune/decay quickly.
    static void voice (V& v, int idx, float tune, float decay)
    {
        v.push_back ({ ParamID::tune (idx),  tune });
        v.push_back ({ ParamID::decay (idx), decay });
    }

    const std::vector<Preset>& getFactoryPresets()
    {
        static std::vector<Preset> presets = []
        {
            std::vector<Preset> out;
            {
                Preset p; p.name = "808 Kit";
                voice (p.values, Drum::Kick, 0.25f, 0.75f);
                voice (p.values, Drum::Snare, 0.45f, 0.4f);
                voice (p.values, Drum::Clap, 0.5f, 0.45f);
                voice (p.values, Drum::ClosedHat, 0.5f, 0.25f);
                voice (p.values, Drum::OpenHat, 0.5f, 0.55f);
                voice (p.values, Drum::Tom, 0.35f, 0.5f);
                voice (p.values, Drum::Rim, 0.5f, 0.4f);
                p.values.push_back ({ ParamID::drive, 0.15f });
                out.push_back (p);
            }
            {
                Preset p; p.name = "909 Punch";
                voice (p.values, Drum::Kick, 0.4f, 0.45f);
                voice (p.values, Drum::Snare, 0.55f, 0.5f);
                voice (p.values, Drum::Clap, 0.6f, 0.4f);
                voice (p.values, Drum::ClosedHat, 0.6f, 0.2f);
                voice (p.values, Drum::OpenHat, 0.6f, 0.5f);
                voice (p.values, Drum::Tom, 0.5f, 0.45f);
                voice (p.values, Drum::Rim, 0.55f, 0.35f);
                p.values.push_back ({ ParamID::drive, 0.3f });
                out.push_back (p);
            }
            {
                Preset p; p.name = "Deep Sub";
                voice (p.values, Drum::Kick, 0.1f, 0.95f);
                voice (p.values, Drum::Snare, 0.35f, 0.55f);
                voice (p.values, Drum::Clap, 0.4f, 0.6f);
                voice (p.values, Drum::ClosedHat, 0.4f, 0.3f);
                voice (p.values, Drum::OpenHat, 0.4f, 0.7f);
                voice (p.values, Drum::Tom, 0.25f, 0.7f);
                voice (p.values, Drum::Rim, 0.45f, 0.5f);
                p.values.push_back ({ ParamID::drive, 0.2f });
                out.push_back (p);
            }
            {
                Preset p; p.name = "Tight & Dry";
                voice (p.values, Drum::Kick, 0.5f, 0.3f);
                voice (p.values, Drum::Snare, 0.6f, 0.25f);
                voice (p.values, Drum::Clap, 0.55f, 0.25f);
                voice (p.values, Drum::ClosedHat, 0.7f, 0.12f);
                voice (p.values, Drum::OpenHat, 0.7f, 0.3f);
                voice (p.values, Drum::Tom, 0.55f, 0.3f);
                voice (p.values, Drum::Rim, 0.6f, 0.2f);
                p.values.push_back ({ ParamID::drive, 0.1f });
                out.push_back (p);
            }
            return out;
        }();
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
