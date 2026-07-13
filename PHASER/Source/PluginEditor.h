#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "AurLookAndFeel.h"
#include "Knob.h"
#include "MeterComponent.h"
class PhaserEditor : public juce::AudioProcessorEditor{
public:
  explicit PhaserEditor(PhaserProcessor&); ~PhaserEditor() override;
  void paint(juce::Graphics&) override; void resized() override;
private:
  void refreshPresetBox(); void applyThemeChoice(int);
  PhaserProcessor& ap; aur::ui::AurLookAndFeel lnf;
  std::array<std::unique_ptr<aur::ui::LabeledKnob>,5> knobs;
  std::vector<std::unique_ptr<juce::TextButton>> toggleBtns;
  std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>> toggleAtt;
  juce::ComboBox presetBox; juce::TextButton bypassButton{"BYPASS"};
  std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAtt;
  aur::ui::MeterComponent inMeter{ap.getMeterState(),aur::ui::MeterComponent::Which::input,"IN"};
  aur::ui::MeterComponent outMeter{ap.getMeterState(),aur::ui::MeterComponent::Which::output,"OUT"};
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PhaserEditor)
};
