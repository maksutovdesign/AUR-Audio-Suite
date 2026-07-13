#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "AurLookAndFeel.h"
#include "Knob.h"
#include "MeterComponent.h"

/** AUR GRIP editor — shared AurvedaUI design system. */
class GripEditor : public juce::AudioProcessorEditor
{
public:
    explicit GripEditor (GripProcessor&);
    ~GripEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void refreshPresetBox();
    void applyThemeChoice (int index);

    GripProcessor& audioProcessor;
    aur::ui::AurLookAndFeel lnf;

    std::unique_ptr<aur::ui::LabeledKnob> threshKnob, ratioKnob, attackKnob, releaseKnob, makeupKnob, mixKnob;

    juce::ComboBox presetBox;
    juce::TextButton bypassButton { "BYPASS" };

    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttachment;

    aur::ui::MeterComponent inMeter  { audioProcessor.getMeterState(), aur::ui::MeterComponent::Which::input,         "IN" };
    aur::ui::MeterComponent grMeter  { audioProcessor.getMeterState(), aur::ui::MeterComponent::Which::gainReduction, "GR" };
    aur::ui::MeterComponent outMeter { audioProcessor.getMeterState(), aur::ui::MeterComponent::Which::output,        "OUT" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GripEditor)
};
