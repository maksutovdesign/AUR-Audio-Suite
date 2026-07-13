#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "AurLookAndFeel.h"
#include "Knob.h"
#include "MeterComponent.h"
#include "SpectrumAnalyzer.h"

/** AUR FORGE editor — CHARACTER-macro channel strip. */
class ForgeEditor : public juce::AudioProcessorEditor
{
public:
    explicit ForgeEditor (ForgeProcessor&);
    ~ForgeEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void refreshPresetBox();
    void applyThemeChoice (int index);

    ForgeProcessor& audioProcessor;
    aur::ui::AurLookAndFeel lnf;

    aur::ui::SpectrumAnalyzer spectrum { audioProcessor.getAnalyzer() };
    std::unique_ptr<aur::ui::LabeledKnob> charK, inputK, hpfK, toneK, outputK;

    juce::ComboBox flavorBox, presetBox;
    juce::TextButton bypassButton { "BYPASS" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> flavorAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>   bypassAtt;

    aur::ui::MeterComponent inMeter { audioProcessor.getMeterState(), aur::ui::MeterComponent::Which::input,         "IN" };
    aur::ui::MeterComponent grMeter { audioProcessor.getMeterState(), aur::ui::MeterComponent::Which::gainReduction, "GR" };
    aur::ui::MeterComponent outMeter{ audioProcessor.getMeterState(), aur::ui::MeterComponent::Which::output,        "OUT" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ForgeEditor)
};
