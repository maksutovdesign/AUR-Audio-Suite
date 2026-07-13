#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "AurLookAndFeel.h"
#include "Knob.h"
#include "SpectrumAnalyzer.h"

/** AUR DENOISE editor — Learn button + spectrum + amount/sensitivity. */
class DenoiseEditor : public juce::AudioProcessorEditor,
                      private juce::Timer
{
public:
    explicit DenoiseEditor (DenoiseProcessor&);
    ~DenoiseEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override { repaint(); }
    void applyThemeChoice (int index);

    DenoiseProcessor& audioProcessor;
    aur::ui::AurLookAndFeel lnf;

    aur::ui::SpectrumAnalyzer spectrum { audioProcessor.getAnalyzer() };
    std::unique_ptr<aur::ui::LabeledKnob> amountK, sensK;

    juce::TextButton learnButton { "LEARN NOISE" };
    juce::TextButton bypassButton { "BYPASS" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAtt;

    juce::Rectangle<int> statusArea;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DenoiseEditor)
};
