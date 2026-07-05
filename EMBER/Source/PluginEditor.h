#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "AurLookAndFeel.h"
#include "Knob.h"
#include "MeterComponent.h"

/** AUR EMBER editor — uses the shared AurvedaUI design system. UI only. */
class EmberEditor : public juce::AudioProcessorEditor
{
public:
    explicit EmberEditor (EmberProcessor&);
    ~EmberEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void refreshPresetBox();

    EmberProcessor& audioProcessor;
    aur::ui::AurLookAndFeel lnf;

    std::unique_ptr<aur::ui::LabeledKnob> driveKnob, inputKnob, mixKnob, toneKnob, outputKnob;

    juce::ComboBox flavorBox, presetBox;
    juce::TextButton bypassButton { "BYPASS" };

    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> flavorAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>   bypassAttachment;

    aur::ui::MeterComponent inMeter  { audioProcessor.getMeterState(), aur::ui::MeterComponent::Which::input,  "IN" };
    aur::ui::MeterComponent outMeter { audioProcessor.getMeterState(), aur::ui::MeterComponent::Which::output, "OUT" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EmberEditor)
};
