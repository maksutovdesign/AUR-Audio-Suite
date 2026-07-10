#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "AurLookAndFeel.h"
#include "Knob.h"
#include "MeterComponent.h"

/** AUR CEIL editor — shared AurvedaUI design system + LUFS readout. */
class CeilEditor : public juce::AudioProcessorEditor,
                   private juce::Timer
{
public:
    explicit CeilEditor (CeilProcessor&);
    ~CeilEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override { repaint(); }
    void refreshPresetBox();
    void applyThemeChoice (int index);

    CeilProcessor& audioProcessor;
    aur::ui::AurLookAndFeel lnf;

    std::unique_ptr<aur::ui::LabeledKnob> gainKnob, ceilingKnob, releaseKnob;

    juce::ComboBox presetBox, themeBox;
    juce::TextButton bypassButton { "BYPASS" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttachment;

    aur::ui::MeterComponent inMeter  { audioProcessor.getMeterState(), aur::ui::MeterComponent::Which::input,         "IN" };
    aur::ui::MeterComponent grMeter  { audioProcessor.getMeterState(), aur::ui::MeterComponent::Which::gainReduction, "GR" };
    aur::ui::MeterComponent outMeter { audioProcessor.getMeterState(), aur::ui::MeterComponent::Which::output,        "OUT" };

    juce::Rectangle<int> lufsArea;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CeilEditor)
};
