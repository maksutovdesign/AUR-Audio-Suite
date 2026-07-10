#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "AurLookAndFeel.h"
#include "Knob.h"
#include "MeterComponent.h"
#include "CorrelationMeter.h"

/** AUR IMAGER editor — shared AurvedaUI design system. */
class ImagerEditor : public juce::AudioProcessorEditor
{
public:
    explicit ImagerEditor (ImagerProcessor&);
    ~ImagerEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void refreshPresetBox();
    void applyThemeChoice (int index);

    ImagerProcessor& audioProcessor;
    aur::ui::AurLookAndFeel lnf;

    std::unique_ptr<aur::ui::LabeledKnob> widthK, monoK, balK;

    juce::ComboBox presetBox, themeBox;
    juce::TextButton bypassButton { "BYPASS" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttachment;

    aur::ui::MeterComponent inMeter  { audioProcessor.getMeterState(), aur::ui::MeterComponent::Which::input,  "L" };
    aur::ui::MeterComponent outMeter { audioProcessor.getMeterState(), aur::ui::MeterComponent::Which::output, "R" };
    aur::ui::CorrelationMeter corr   { audioProcessor.getMeterState() };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ImagerEditor)
};
