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
        att = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (a, id, box);
    }
    void resized() override { auto b = getLocalBounds(); label.setBounds (b.removeFromTop (16)); box.setBounds (b.reduced (2, 8).withTrimmedTop (10)); }
private:
    juce::Label label; juce::ComboBox box;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> att;
};

/** Draggable XY pad bound to two float parameters. */
class XYPad : public juce::Component
{
public:
    XYPad (juce::AudioProcessorValueTreeState& s, const juce::String& xId, const juce::String& yId)
        : px (s.getParameter (xId)), py (s.getParameter (yId)) {}

    void paint (juce::Graphics& g) override
    {
        const auto& t = aur::ui::theme();
        g.setColour (t.panel);
        g.fillRoundedRectangle (getLocalBounds().toFloat(), t.cornerRadius);
        g.setColour (t.inkDim.withAlpha (0.4f));
        g.drawLine ((float) getWidth() / 2, 6.0f, (float) getWidth() / 2, (float) getHeight() - 6.0f);
        g.drawLine (6.0f, (float) getHeight() / 2, (float) getWidth() - 6.0f, (float) getHeight() / 2);
        const float x = px->getValue() * (float) (getWidth() - 24) + 12.0f;
        const float y = (1.0f - py->getValue()) * (float) (getHeight() - 24) + 12.0f;
        g.setColour (t.precisionBright);
        g.fillEllipse (x - 7, y - 7, 14, 14);
        g.setColour (t.precision.withAlpha (0.3f));
        g.drawEllipse (x - 11, y - 11, 22, 22, 1.5f);
    }
    void mouseDown (const juce::MouseEvent& e) override { drag (e); }
    void mouseDrag (const juce::MouseEvent& e) override { drag (e); }
private:
    void drag (const juce::MouseEvent& e)
    {
        const float nx = juce::jlimit (0.0f, 1.0f, ((float) e.x - 12.0f) / (float) (getWidth() - 24));
        const float ny = juce::jlimit (0.0f, 1.0f, 1.0f - ((float) e.y - 12.0f) / (float) (getHeight() - 24));
        px->setValueNotifyingHost (nx);
        py->setValueNotifyingHost (ny);
        repaint();
    }
    juce::RangedAudioParameter *px, *py;
};

class VectorEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit VectorEditor (VectorProcessor&);
    ~VectorEditor() override;
    void paint (juce::Graphics&) override;
    void resized() override;
private:
    void timerCallback() override { repaint(); }
    void refreshPresetBox();

    struct Cell { juce::String id, caption; bool combo; };
    std::vector<Cell> cells;
    std::vector<std::unique_ptr<juce::Component>> controls;

    VectorProcessor& audioProcessor;
    aur::ui::AurLookAndFeel lnf;
    std::unique_ptr<XYPad> xyPad;
    juce::ComboBox presetBox;
    aur::ui::MeterComponent outMeter { audioProcessor.getMeterState(), aur::ui::MeterComponent::Which::output, "OUT" };
    juce::MidiKeyboardComponent keyboard;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VectorEditor)
};
