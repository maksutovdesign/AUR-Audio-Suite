#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "AurLookAndFeel.h"
#include "Knob.h"
#include "MeterComponent.h"

class Kit9Editor : public juce::AudioProcessorEditor,
                    private juce::Timer
{
public:
    explicit Kit9Editor (Kit9Processor&);
    ~Kit9Editor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override { repaint(); }
    void refreshPresetBox();

    Kit9Processor& audioProcessor;
    aur::ui::AurLookAndFeel lnf;

    struct Row
    {
        juce::TextButton pad;
        std::unique_ptr<aur::ui::LabeledKnob> level, tune, decay;
    };
    std::array<Row, Drum::Count> rows;

    std::unique_ptr<aur::ui::LabeledKnob> driveKnob, volumeKnob;
    juce::ComboBox presetBox;
    aur::ui::MeterComponent outMeter { audioProcessor.getMeterState(), aur::ui::MeterComponent::Which::output, "OUT" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Kit9Editor)
};
