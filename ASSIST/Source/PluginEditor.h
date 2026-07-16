#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "AurLookAndFeel.h"
#include "Knob.h"
#include "SpectrumAnalyzer.h"

/** AUR ASSIST editor — Master Assistant with an Analyse button and readouts. */
class AssistEditor : public juce::AudioProcessorEditor,
                     private juce::Timer
{
public:
    explicit AssistEditor (AssistProcessor&);
    ~AssistEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override { repaint(); }
    void applyThemeChoice (int index);

    AssistProcessor& audioProcessor;
    aur::ui::AurLookAndFeel lnf;

    aur::ui::SpectrumAnalyzer spectrum { audioProcessor.getAnalyzer() };
    std::unique_ptr<aur::ui::LabeledKnob> intensityK, ceilingK;

    juce::TextButton assistButton { "ASSIST" };
    juce::TextButton bypassButton { "BYPASS" };
    juce::ComboBox targetBox, toneBox;

    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> targetAtt, toneAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>   bypassAtt;

    juce::Rectangle<int> readout;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AssistEditor)
};
