#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "AurLookAndFeel.h"
#include "Knob.h"
#include "MeterComponent.h"

/** AUR CLARITY editor — shared AurvedaUI design system. */
class ClarityEditor : public juce::AudioProcessorEditor
{
public:
    explicit ClarityEditor (ClarityProcessor&);
    ~ClarityEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void refreshPresetBox();
    void applyThemeChoice (int index);

    ClarityProcessor& audioProcessor;
    aur::ui::AurLookAndFeel lnf;

    std::unique_ptr<aur::ui::LabeledKnob> depthKnob, sensKnob, sharpKnob, mixKnob;

    juce::ComboBox presetBox, themeBox;
    juce::TextButton deltaButton { "DELTA" };
    juce::TextButton bypassButton { "BYPASS" };

    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> deltaAttachment, bypassAttachment;

    aur::ui::MeterComponent inMeter  { audioProcessor.getMeterState(), aur::ui::MeterComponent::Which::input,  "IN" };
    aur::ui::MeterComponent outMeter { audioProcessor.getMeterState(), aur::ui::MeterComponent::Which::output, "OUT" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ClarityEditor)
};
