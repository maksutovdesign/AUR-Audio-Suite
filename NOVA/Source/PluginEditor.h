#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "PluginProcessor.h"
#include "AurLookAndFeel.h"
#include "Knob.h"
#include "MeterComponent.h"

/** Caption + combo bound to a choice parameter. */
class LabeledCombo : public juce::Component
{
public:
    LabeledCombo (juce::AudioProcessorValueTreeState& apvts,
                  const juce::String& paramID, const juce::String& caption)
    {
        label.setText (caption, juce::dontSendNotification);
        label.setJustificationType (juce::Justification::centred);
        label.setFont (aur::ui::monoFont (aur::ui::theme().fsLabel, true));
        label.setColour (juce::Label::textColourId, aur::ui::theme().inkMute);
        addAndMakeVisible (label);
        addAndMakeVisible (box);
        if (auto* pc = dynamic_cast<juce::AudioParameterChoice*> (apvts.getParameter (paramID)))
            box.addItemList (pc->choices, 1);
        attachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (apvts, paramID, box);
    }
    void resized() override
    {
        auto b = getLocalBounds();
        label.setBounds (b.removeFromTop (16));
        box.setBounds (b.reduced (2, 8).withTrimmedTop (10));
    }
private:
    juce::Label label;
    juce::ComboBox box;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> attachment;
};

class NovaEditor : public juce::AudioProcessorEditor,
                   private juce::Timer
{
public:
    explicit NovaEditor (NovaProcessor&);
    ~NovaEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override { repaint(); }
    void refreshPresetBox();
    void buildControls();

    struct Cell { juce::String id; juce::String caption; bool combo; };
    struct Section { juce::String title; std::vector<Cell> cells; int column; };

    NovaProcessor& audioProcessor;
    aur::ui::AurLookAndFeel lnf;

    std::vector<Section> sections;
    std::vector<std::unique_ptr<juce::Component>> controls;
    std::vector<juce::Component*> cellByOrder;
    std::vector<std::pair<juce::String, juce::Rectangle<int>>> titleRects;

    juce::ComboBox presetBox;
    aur::ui::MeterComponent outMeter { audioProcessor.getMeterState(), aur::ui::MeterComponent::Which::output, "OUT" };
    juce::MidiKeyboardComponent keyboard;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NovaEditor)
};
