#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "AurLookAndFeel.h"
#include "Knob.h"
#include "EqDisplay.h"

/** AUR PRISM editor — spectrum + EQ curve hero, grouped band knobs. */
class PrismEditor : public juce::AudioProcessorEditor
{
public:
    explicit PrismEditor (PrismProcessor&);
    ~PrismEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void refreshPresetBox();
    void applyThemeChoice (int index);

    PrismProcessor& audioProcessor;
    aur::ui::AurLookAndFeel lnf;

    std::unique_ptr<aur::ui::LabeledKnob> hpK, lsFK, lsGK, bFK, bGK, bQK, hsFK, hsGK, lpK;
    std::unique_ptr<EqDisplay> display;

    juce::ComboBox presetBox, themeBox;
    juce::TextButton bypassButton { "BYPASS" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PrismEditor)
};
