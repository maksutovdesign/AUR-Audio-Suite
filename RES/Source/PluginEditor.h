#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "PluginProcessor.h"
#include "AurLookAndFeel.h"
#include "Knob.h"
#include "MeterComponent.h"

class LabeledCombo : public juce::Component
{
public:
    LabeledCombo (juce::AudioProcessorValueTreeState& a, const juce::String& id, const juce::String& cap)
    {
        label.setText (cap, juce::dontSendNotification);
        label.setJustificationType (juce::Justification::centred);
        label.setFont (aur::ui::monoFont (aur::ui::theme().fsLabel, true));
        label.setColour (juce::Label::textColourId, aur::ui::theme().inkMute);
        addAndMakeVisible (label); addAndMakeVisible (box);
        if (auto* pc = dynamic_cast<juce::AudioParameterChoice*> (a.getParameter (id)))
            box.addItemList (pc->choices, 1);
        att = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (a, id, box);
    }
    void resized() override { auto b = getLocalBounds(); label.setBounds (b.removeFromTop (16)); box.setBounds (b.reduced (2, 8).withTrimmedTop (10)); }
private:
    juce::Label label; juce::ComboBox box;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> att;
};

class ResEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit ResEditor (ResProcessor&);
    ~ResEditor() override;
    void paint (juce::Graphics&) override;
    void resized() override;
private:
    void timerCallback() override { repaint(); }
    void refreshPresetBox();

    struct Cell { juce::String id, caption; bool combo; };
    std::vector<Cell> cells;
    std::vector<std::unique_ptr<juce::Component>> controls;

    ResProcessor& audioProcessor;
    aur::ui::AurLookAndFeel lnf;
    juce::ComboBox presetBox;
    aur::ui::MeterComponent outMeter { audioProcessor.getMeterState(), aur::ui::MeterComponent::Which::output, "OUT" };
    juce::MidiKeyboardComponent keyboard;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ResEditor)
};
