#include "PluginEditor.h"
#include "Parameters.h"
#include "Presets.h"
#include "Theme.h"
#include "Branding.h"

using namespace aur::ui;

OrganEditor::OrganEditor (OrganProcessor& p)
    : AudioProcessorEditor (p), audioProcessor (p),
      keyboard (p.getKeyboardState(), juce::MidiKeyboardComponent::horizontalKeyboard)
{
    setLookAndFeel (&lnf);
    auto& a = audioProcessor.getAPVTS();
    cells = {
        { ParamID::bar (0), "16'", false }, { ParamID::bar (1), "5.1/3'", false }, { ParamID::bar (2), "8'", false }, { ParamID::bar (3), "4'", false }, { ParamID::bar (4), "2.2/3'", false }, { ParamID::bar (5), "2'", false }, { ParamID::bar (6), "1.3/5'", false }, { ParamID::bar (7), "1.1/3'", false }, { ParamID::bar (8), "1'", false },
        { ParamID::perc, "PERC", false }, { ParamID::vibrato, "VIB", false }, { ParamID::rotary, "ROTARY", false },
        { ParamID::drive, "DRIVE", false }, { ParamID::volume, "VOLUME", false },
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

OrganEditor::~OrganEditor() { stopTimer(); setLookAndFeel (nullptr); }

void OrganEditor::refreshPresetBox()
{
    presetBox.clear (juce::dontSendNotification);
    int id = 1;
    for (const auto& pr : Presets::getFactoryPresets()) presetBox.addItem (pr.name, id++);
    presetBox.setSelectedId (audioProcessor.getCurrentProgram() + 1, juce::dontSendNotification);
}

void OrganEditor::paint (juce::Graphics& g)
{
    const auto& t = theme();
    g.fillAll (t.ground);
    const auto cx = (float) getWidth() * 0.5f;
    juce::ColourGradient glow (t.precision.withAlpha (0.10f), cx, 110.0f, t.ground.withAlpha (0.0f), cx, 380.0f, true);
    g.setGradientFill (glow); g.fillRect (getLocalBounds());
    drawBrandHeader (g, { 20, 16, 380, 60 }, "ORGAN", "Tonewheel drawbar organ");
}

void OrganEditor::resized()
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
