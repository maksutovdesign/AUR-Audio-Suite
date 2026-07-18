#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "PluginProcessor.h"
#include "AurLookAndFeel.h"
#include "Knob.h"
#include "MeterComponent.h"

class PluckEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit PluckEditor (PluckProcessor&);
    ~PluckEditor() override;
    void paint (juce::Graphics&) override;
    void resized() override;
private:
    void timerCallback() override { repaint(); }
    void refreshPresetBox();

    struct Cell { juce::String id, caption; };
    std::vector<Cell> cells;
    std::vector<std::unique_ptr<aur::ui::LabeledKnob>> knobs;

    PluckProcessor& audioProcessor;
    aur::ui::AurLookAndFeel lnf;
    juce::ComboBox presetBox;
    aur::ui::MeterComponent outMeter { audioProcessor.getMeterState(), aur::ui::MeterComponent::Which::output, "OUT" };
    juce::MidiKeyboardComponent keyboard;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluckEditor)
};
