#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <vector>

namespace Presets
{
    struct Preset
    {
        juce::String name;
        std::vector<std::pair<juce::String, float>> values;
    };

    const std::vector<Preset>& getFactoryPresets();
    void apply (int index, juce::AudioProcessorValueTreeState& apvts);
}
