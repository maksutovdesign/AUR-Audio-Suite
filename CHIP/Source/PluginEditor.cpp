#include "PluginEditor.h"
#include "Parameters.h"
#include "Presets.h"
#include "Theme.h"
#include "Branding.h"

using namespace aur::ui;

ChipEditor::ChipEditor (ChipProcessor& p)
    : AudioProcessorEditor (p), audioProcessor (p),
      keyboard (p.getKeyboardState(), juce::MidiKeyboardComponent::horizontalKeyboard)
{
    setLookAndFeel (&lnf);
    auto& a = audioProcessor.getAPVTS();
    cells = {
        { ParamID::shape, "SHAPE", true }, { ParamID::crush, "CRUSH", false }, { ParamID::vibrato, "VIB", false },
        { ParamID::attack, "ATK", false }, { ParamID::decay, "DEC", false }, { ParamID::sustain, "SUS", false }, { ParamID::release, "REL", false },
        { ParamID::volume, "VOLUME", false },
    };
    for (auto& c : cells)
    {
        std::unique_ptr<juce::Component> comp;
        if (c.combo) comp = std::make_unique<LabeledCombo> (a, c.id, c.caption);
        else         comp = std::make_unique<LabeledKnob>  (a, c.id, c.caption);
        addAndMakeVisible (*comp);
        controls.push_back (std::move (comp));
    }

    addAndMakeVisible (presetBox);
    refreshPresetBox();
    presetBox.onChange = [this] { const auto i = presetBox.getSelectedId() - 1; if (i >= 0) audioProcessor.setCurrentProgram (i); };
    addAndMakeVisible (outMeter);
    keyboard.setKeyWidth (20.0f); keyboard.setLowestVisibleKey (36);
    addAndMakeVisible (keyboard);

    startTimerHz (15);
    setSize (720, 400);
}

ChipEditor::~ChipEditor() { stopTimer(); setLookAndFeel (nullptr); }

void ChipEditor::refreshPresetBox()
{
    presetBox.clear (juce::dontSendNotification);
    int id = 1;
    for (const auto& pr : Presets::getFactoryPresets()) presetBox.addItem (pr.name, id++);
    presetBox.setSelectedId (audioProcessor.getCurrentProgram() + 1, juce::dontSendNotification);
}

void ChipEditor::paint (juce::Graphics& g)
{
    const auto& t = theme();
    g.fillAll (t.ground);
    const auto cx = (float) getWidth() * 0.5f;
    juce::ColourGradient glow (t.precision.withAlpha (0.10f), cx, 110.0f, t.ground.withAlpha (0.0f), cx, 380.0f, true);
    g.setGradientFill (glow); g.fillRect (getLocalBounds());
    drawBrandHeader (g, { 20, 16, 380, 60 }, "CHIP", "Chiptune / 8-bit synth");
}

void ChipEditor::resized()
{
    auto area = getLocalBounds().reduced (18);
    auto header = area.removeFromTop (56);
    header.removeFromLeft (250);
    presetBox.setBounds (header.removeFromRight (160).reduced (4, 14));
    outMeter.setBounds  (header.removeFromRight (60).reduced (4, 6));

    keyboard.setBounds (area.removeFromBottom (74).reduced (0, 6));
    area.removeFromTop (6);

    const int cellW = 86, cellH = 90;
    const int perRow = juce::jmax (1, area.getWidth() / cellW);
    for (size_t i = 0; i < controls.size(); ++i)
    {
        const int row = (int) i / perRow, col = (int) i % perRow;
        controls[i]->setBounds (area.getX() + col * cellW, area.getY() + row * cellH, cellW, cellH);
    }
}
