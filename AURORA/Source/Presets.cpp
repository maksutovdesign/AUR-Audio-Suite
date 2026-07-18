#include "Presets.h"
#include "Parameters.h"

namespace Presets
{
    using V = std::vector<std::pair<juce::String, float>>;

    const std::vector<Preset>& getFactoryPresets()
    {
        static const std::vector<Preset> presets =
        {
            { "Init Saw", V{
                { ParamID::osc1shape, 0 }, { ParamID::osc1level, 0.8f }, { ParamID::osc2level, 0.f },
                { ParamID::cutoff, 12000.f }, { ParamID::resonance, 0.15f }, { ParamID::envamt, 0.4f },
                { ParamID::unison, 1 }, { ParamID::drive, 0.15f }, { ParamID::volume, -8.f } } },

            { "Fat Unison Lead", V{
                { ParamID::osc1shape, 0 }, { ParamID::osc1level, 0.9f },
                { ParamID::osc2shape, 0 }, { ParamID::osc2level, 0.7f }, { ParamID::osc2fine, 7.f },
                { ParamID::cutoff, 9000.f }, { ParamID::resonance, 0.2f }, { ParamID::envamt, 0.35f },
                { ParamID::unison, 7 }, { ParamID::detune, 0.4f }, { ParamID::spread, 0.8f },
                { ParamID::aatk, 0.005f }, { ParamID::adec, 0.6f }, { ParamID::asus, 0.7f }, { ParamID::arel, 0.4f },
                { ParamID::drive, 0.3f }, { ParamID::volume, -10.f } } },

            { "Moog Sub Bass", V{
                { ParamID::osc1shape, 0 }, { ParamID::osc1level, 0.9f },
                { ParamID::sublevel, 0.6f }, { ParamID::cutoff, 1400.f }, { ParamID::resonance, 0.3f },
                { ParamID::fdrive, 3.f }, { ParamID::envamt, 0.55f }, { ParamID::keytrack, 0.3f },
                { ParamID::fatk, 0.002f }, { ParamID::fdec, 0.25f }, { ParamID::fsus, 0.1f },
                { ParamID::aatk, 0.002f }, { ParamID::adec, 0.4f }, { ParamID::asus, 0.9f }, { ParamID::arel, 0.15f },
                { ParamID::unison, 1 }, { ParamID::drive, 0.35f }, { ParamID::volume, -7.f } } },

            { "Warm Pad", V{
                { ParamID::osc1shape, 0 }, { ParamID::osc1level, 0.7f },
                { ParamID::osc2shape, 2 }, { ParamID::osc2level, 0.6f }, { ParamID::osc2coarse, -12.f },
                { ParamID::cutoff, 4500.f }, { ParamID::resonance, 0.12f }, { ParamID::envamt, 0.3f },
                { ParamID::fatk, 0.8f }, { ParamID::fdec, 1.5f }, { ParamID::fsus, 0.5f }, { ParamID::frel, 2.f },
                { ParamID::aatk, 0.6f }, { ParamID::adec, 1.f }, { ParamID::asus, 0.8f }, { ParamID::arel, 1.8f },
                { ParamID::unison, 5 }, { ParamID::detune, 0.3f }, { ParamID::spread, 0.9f },
                { ParamID::lforate, 0.3f }, { ParamID::lfo2cut, 0.25f },
                { ParamID::drive, 0.2f }, { ParamID::volume, -9.f } } },

            { "Acid Squelch", V{
                { ParamID::osc1shape, 0 }, { ParamID::osc1level, 0.9f },
                { ParamID::cutoff, 700.f }, { ParamID::resonance, 0.75f }, { ParamID::fdrive, 4.f },
                { ParamID::envamt, 0.7f }, { ParamID::keytrack, 0.4f },
                { ParamID::fatk, 0.002f }, { ParamID::fdec, 0.2f }, { ParamID::fsus, 0.f },
                { ParamID::aatk, 0.002f }, { ParamID::adec, 0.3f }, { ParamID::asus, 0.6f }, { ParamID::arel, 0.1f },
                { ParamID::unison, 1 }, { ParamID::drive, 0.4f }, { ParamID::volume, -9.f } } },

            { "S&H Movement", V{
                { ParamID::osc1shape, 1 }, { ParamID::osc1level, 0.8f }, { ParamID::osc1pw, 0.35f },
                { ParamID::cutoff, 3000.f }, { ParamID::resonance, 0.4f }, { ParamID::envamt, 0.2f },
                { ParamID::lforate, 8.f }, { ParamID::lfoshape, 4 }, { ParamID::lfo2cut, 0.6f },
                { ParamID::unison, 3 }, { ParamID::detune, 0.2f }, { ParamID::drive, 0.25f }, { ParamID::volume, -9.f } } },
        };
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
