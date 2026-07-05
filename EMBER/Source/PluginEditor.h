#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "UI/MoltenLookAndFeel.h"
#include "UI/Knob.h"
#include "UI/MeterComponent.h"

/** AUR EMBER editor — MOLTEN design language. UI only, binds via APVTS. */
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
    MoltenLookAndFeel lnf;

    std::unique_ptr<LabeledKnob> driveKnob, inputKnob, mixKnob, toneKnob, outputKnob;

    juce::ComboBox flavorBox, presetBox;
    juce::TextButton bypassButton { "BYPASS" };

    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> flavorAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>   bypassAttachment;

    MeterComponent inMeter  { audioProcessor.getMeterState(), MeterComponent::Which::input,  "IN" };
    MeterComponent outMeter { audioProcessor.getMeterState(), MeterComponent::Which::output, "OUT" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EmberEditor)
};
