#include "Presets.h"
#include "Parameters.h"

namespace Presets
{
    using V = std::vector<std::pair<juce::String, float>>;

    static void op (V& v, int i, float ratio, float level, float a, float d, float s, float r)
    {
        v.push_back ({ ParamID::ratio (i), ratio });
        v.push_back ({ ParamID::level (i), level });
        v.push_back ({ ParamID::atk (i), a });
        v.push_back ({ ParamID::dec (i), d });
        v.push_back ({ ParamID::sus (i), s });
        v.push_back ({ ParamID::rel (i), r });
    }

    const std::vector<Preset>& getFactoryPresets()
    {
        static std::vector<Preset> presets = []
        {
            std::vector<Preset> out;
            {
                Preset p; p.name = "E-Piano";
                op (p.values, 0, 1.0f, 0.9f, 0.002f, 1.2f, 0.0f, 0.4f);   // carrier
                op (p.values, 1, 1.0f, 0.45f, 0.002f, 0.5f, 0.0f, 0.3f);  // mod
                op (p.values, 2, 14.0f, 0.6f, 0.002f, 0.12f, 0.0f, 0.2f); // 2nd carrier (attack tine)
                op (p.values, 3, 1.0f, 0.3f, 0.002f, 0.1f, 0.0f, 0.2f);
                p.values.push_back ({ ParamID::algo, 1 });                 // Twin Stack
                p.values.push_back ({ ParamID::drive, 0.12f });
                out.push_back (p);
            }
            {
                Preset p; p.name = "FM Bell";
                op (p.values, 0, 1.0f, 0.9f, 0.001f, 2.5f, 0.0f, 2.0f);
                op (p.values, 1, 3.5f, 0.7f, 0.001f, 2.0f, 0.0f, 1.5f);
                op (p.values, 2, 1.0f, 0.0f, 0.01f, 1.f, 0.f, 1.f);
                op (p.values, 3, 7.0f, 0.4f, 0.001f, 1.5f, 0.0f, 1.f);
                p.values.push_back ({ ParamID::algo, 0 });                 // Chain
                p.values.push_back ({ ParamID::drive, 0.08f });
                out.push_back (p);
            }
            {
                Preset p; p.name = "FM Bass";
                op (p.values, 0, 1.0f, 0.95f, 0.002f, 0.4f, 0.6f, 0.15f);
                op (p.values, 1, 1.0f, 0.55f, 0.002f, 0.25f, 0.2f, 0.15f);
                op (p.values, 2, 2.0f, 0.0f, 0.01f, 0.3f, 0.f, 0.2f);
                op (p.values, 3, 1.0f, 0.0f, 0.01f, 0.3f, 0.f, 0.2f);
                p.values.push_back ({ ParamID::algo, 0 });
                p.values.push_back ({ ParamID::drive, 0.3f });
                out.push_back (p);
            }
            {
                Preset p; p.name = "Glass Pad";
                op (p.values, 0, 1.0f, 0.8f, 0.6f, 1.5f, 0.8f, 1.8f);
                op (p.values, 1, 2.0f, 0.35f, 0.8f, 1.5f, 0.6f, 1.8f);
                op (p.values, 2, 3.0f, 0.5f, 0.9f, 1.5f, 0.7f, 2.0f);
                op (p.values, 3, 4.0f, 0.25f, 1.0f, 1.5f, 0.5f, 2.0f);
                p.values.push_back ({ ParamID::algo, 4 });                 // Parallel
                p.values.push_back ({ ParamID::drive, 0.1f });
                out.push_back (p);
            }
            {
                Preset p; p.name = "Metallic Stab";
                op (p.values, 0, 1.0f, 0.9f, 0.001f, 0.5f, 0.0f, 0.3f);
                op (p.values, 1, 1.41f, 0.7f, 0.001f, 0.4f, 0.0f, 0.3f);
                op (p.values, 2, 3.14f, 0.6f, 0.001f, 0.35f, 0.0f, 0.25f);
                op (p.values, 3, 5.0f, 0.5f, 0.001f, 0.3f, 0.0f, 0.2f);
                p.values.push_back ({ ParamID::algo, 2 });                 // 3→Carrier
                p.values.push_back ({ ParamID::feedback, 0.3f });
                p.values.push_back ({ ParamID::drive, 0.2f });
                out.push_back (p);
            }
            {
                Preset p; p.name = "Additive Organ";
                op (p.values, 0, 1.0f, 0.7f, 0.005f, 0.5f, 1.0f, 0.2f);
                op (p.values, 1, 2.0f, 0.5f, 0.005f, 0.5f, 1.0f, 0.2f);
                op (p.values, 2, 3.0f, 0.35f, 0.005f, 0.5f, 1.0f, 0.2f);
                op (p.values, 3, 4.0f, 0.25f, 0.005f, 0.5f, 1.0f, 0.2f);
                p.values.push_back ({ ParamID::algo, 5 });                 // Additive
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
