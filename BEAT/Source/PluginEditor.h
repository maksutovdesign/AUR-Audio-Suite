#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "AurLookAndFeel.h"
#include "Knob.h"
#include "MeterComponent.h"

class BeatEditor : public juce::AudioProcessorEditor,
                    private juce::Timer
{
public:
    explicit BeatEditor (BeatProcessor&);
    ~BeatEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override { repaint(); }
    void refreshPresetBox();

    BeatProcessor& audioProcessor;
    aur::ui::AurLookAndFeel lnf;

    struct Row
    {
        juce::TextButton pad;
        std::unique_ptr<aur::ui::LabeledKnob> level, tune, decay;
    };
    std::array<Row, Drum::Count> rows;

    std::unique_ptr<aur::ui::LabeledKnob> driveKnob, volumeKnob;
    std::array<std::array<juce::TextButton, 16>, Drum::Count> stepBtn;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>> stepAtt;
    juce::ComboBox presetBox;
    aur::ui::MeterComponent outMeter { audioProcessor.getMeterState(), aur::ui::MeterComponent::Which::output, "OUT" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BeatEditor)
};
