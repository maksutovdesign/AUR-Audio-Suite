#include "Presets.h"
#include "Parameters.h"

namespace Presets
{
    using V = std::vector<std::pair<juce::String, float>>;
    static void bars (V& v, const float b[9]) { for (int i = 0; i < 9; ++i) v.push_back ({ ParamID::bar (i), b[i] }); }

    const std::vector<Preset>& getFactoryPresets()
    {
        static std::vector<Preset> presets = []
        {
            std::vector<Preset> out;
            { Preset p; p.name = "Full Gospel"; const float b[9] = { .9f,.8f,.9f,.8f,.7f,.6f,.5f,.4f,.6f }; bars (p.values, b);
              p.values.push_back ({ ParamID::perc, 0.f }); p.values.push_back ({ ParamID::rotary, 0.7f }); p.values.push_back ({ ParamID::drive, 0.4f }); out.push_back (p); }
            { Preset p; p.name = "Jazz Trio"; const float b[9] = { .9f,0.f,.8f,.6f,0.f,0.f,0.f,0.f,.3f }; bars (p.values, b);
              p.values.push_back ({ ParamID::perc, 0.6f }); p.values.push_back ({ ParamID::rotary, 0.3f }); out.push_back (p); }
            { Preset p; p.name = "Rock 888"; const float b[9] = { .9f,.9f,.9f,0.f,0.f,0.f,0.f,0.f,0.f }; bars (p.values, b);
              p.values.push_back ({ ParamID::drive, 0.55f }); p.values.push_back ({ ParamID::rotary, 0.8f }); out.push_back (p); }
            { Preset p; p.name = "Flute Stop"; const float b[9] = { 0.f,0.f,.9f,.4f,0.f,.2f,0.f,0.f,0.f }; bars (p.values, b);
              p.values.push_back ({ ParamID::vibrato, 0.4f }); p.values.push_back ({ ParamID::rotary, 0.15f }); out.push_back (p); }
            return out;
        }();
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
