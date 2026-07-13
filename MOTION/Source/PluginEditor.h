#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "AurLookAndFeel.h"
#include "Knob.h"
#include "SpectrumAnalyzer.h"

/** AUR MOTION editor — spectrum hero + 3 dynamic bands. */
class MotionEditor : public juce::AudioProcessorEditor
{
public:
    explicit MotionEditor (MotionProcessor&);
    ~MotionEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void refreshPresetBox();
    void applyThemeChoice (int index);

    MotionProcessor& audioProcessor;
    aur::ui::AurLookAndFeel lnf;

    std::array<std::unique_ptr<aur::ui::LabeledKnob>, 10> knobs;
    aur::ui::SpectrumAnalyzer spectrum { audioProcessor.getAnalyzer() };

    juce::ComboBox presetBox;
    juce::TextButton bypassButton { "BYPASS" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MotionEditor)
};
