#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "AurLookAndFeel.h"
#include "Knob.h"
#include "MeterComponent.h"

/** AUR DEESS editor — shared AurvedaUI design system. */
class DeessEditor : public juce::AudioProcessorEditor
{
public:
    explicit DeessEditor (DeessProcessor&);
    ~DeessEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void refreshPresetBox();
    void applyThemeChoice (int index);

    DeessProcessor& audioProcessor;
    aur::ui::AurLookAndFeel lnf;

    std::unique_ptr<aur::ui::LabeledKnob> freqK, thrK, rangeK;

    juce::ComboBox presetBox, themeBox;
    juce::TextButton listenButton { "LISTEN" };
    juce::TextButton bypassButton { "BYPASS" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> listenAtt, bypassAtt;

    aur::ui::MeterComponent inMeter  { audioProcessor.getMeterState(), aur::ui::MeterComponent::Which::input,         "IN" };
    aur::ui::MeterComponent grMeter  { audioProcessor.getMeterState(), aur::ui::MeterComponent::Which::gainReduction, "GR" };
    aur::ui::MeterComponent outMeter { audioProcessor.getMeterState(), aur::ui::MeterComponent::Which::output,        "OUT" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DeessEditor)
};
