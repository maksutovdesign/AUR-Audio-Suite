#include "PluginEditor.h"
#include "Parameters.h"
#include "Presets.h"
#include "Theme.h"
#include "Branding.h"

using namespace aur::ui;

PluckEditor::PluckEditor (PluckProcessor& p)
    : AudioProcessorEditor (p), audioProcessor (p),
      keyboard (p.getKeyboardState(), juce::MidiKeyboardComponent::horizontalKeyboard)
{
    setLookAndFeel (&lnf);
    auto& a = audioProcessor.getAPVTS();
    cells = { { ParamID::damp, "DAMP" }, { ParamID::sustain, "SUSTAIN" }, { ParamID::bright, "BRIGHT" },
              { ParamID::spread, "SPREAD" }, { ParamID::drive, "DRIVE" }, { ParamID::volume, "VOLUME" } };
    for (auto& c : cells) { auto k = std::make_unique<LabeledKnob> (a, c.id, c.caption); addAndMakeVisible (*k); knobs.push_back (std::move (k)); }

    addAndMakeVisible (presetBox);
    refreshPresetBox();
    presetBox.onChange = [this] { const auto i = presetBox.getSelectedId() - 1; if (i >= 0) audioProcessor.setCurrentProgram (i); };
    addAndMakeVisible (outMeter);
    keyboard.setKeyWidth (20.0f); keyboard.setLowestVisibleKey (36);
    addAndMakeVisible (keyboard);

    startTimerHz (15);
    setSize (640, 360);
}

PluckEditor::~PluckEditor() { stopTimer(); setLookAndFeel (nullptr); }

void PluckEditor::refreshPresetBox()
{
    presetBox.clear (juce::dontSendNotification);
    int id = 1;
    for (const auto& pr : Presets::getFactoryPresets()) presetBox.addItem (pr.name, id++);
    presetBox.setSelectedId (audioProcessor.getCurrentProgram() + 1, juce::dontSendNotification);
}

void PluckEditor::paint (juce::Graphics& g)
{
    const auto& t = theme();
    g.fillAll (t.ground);
    const auto cx = (float) getWidth() * 0.5f;
    juce::ColourGradient glow (t.precision.withAlpha (0.10f), cx, 110.0f, t.ground.withAlpha (0.0f), cx, 360.0f, true);
    g.setGradientFill (glow); g.fillRect (getLocalBounds());
    drawBrandHeader (g, { 20, 16, 380, 60 }, "PLUCK", "Karplus-Strong string synth");
}

void PluckEditor::resized()
{
    auto area = getLocalBounds().reduced (18);
    auto header = area.removeFromTop (56);
    header.removeFromLeft (250);
    presetBox.setBounds (header.removeFromRight (160).reduced (4, 14));
    outMeter.setBounds  (header.removeFromRight (60).reduced (4, 6));

    keyboard.setBounds (area.removeFromBottom (74).reduced (0, 6));
    area.removeFromTop (10);

    const int perRow = juce::jmax (1, (int) knobs.size());
    const int cellW = area.getWidth() / perRow;
    for (size_t i = 0; i < knobs.size(); ++i)
        knobs[i]->setBounds (area.getX() + (int) i * cellW, area.getY(), cellW, juce::jmin (120, area.getHeight()));
}
